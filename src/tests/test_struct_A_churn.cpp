#include "hibbard_bst.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace {

using churn::DeletionPolicy;
using churn::HibbardBst;
using Key = HibbardBst::Key;

constexpr std::uint64_t workload_seed = 7103;
constexpr std::uint64_t policy_seed = 8971;

Key fresh_key(std::mt19937_64& random, std::unordered_set<Key>& present) {
    for (;;) {
        const Key candidate = static_cast<Key>(random() >> 1);
        if (present.insert(candidate).second) {
            return candidate;
        }
    }
}

void checkpoint(const HibbardBst& successor, const HibbardBst& mixed,
                std::vector<Key> expected, std::uint64_t pair) {
    std::sort(expected.begin(), expected.end());

    if (!successor.validate() || !mixed.validate() ||
        successor.inorder() != expected || mixed.inorder() != expected) {
        throw std::runtime_error(
            "checkpoint audit failed at pair " + std::to_string(pair)
        );
    }

    const auto final_pair = static_cast<std::uint64_t>(expected.size()) * expected.size();
    if (pair == 0 || pair == final_pair) {
        for (const auto* tree : {&successor, &mixed}) {
            const auto metrics = tree->metrics();
            const auto* name = tree == &successor ? "successor" : "random";
            std::cout << expected.size() << ',' << pair << ',' << name << ','
                      << metrics.height << ',' << metrics.average_depth() << '\n';
        }
    }
}

void verify_churn(std::size_t n) {
    HibbardBst successor(DeletionPolicy::AlwaysSuccessor);
    HibbardBst mixed(DeletionPolicy::RandomPredecessorOrSuccessor, policy_seed);

    std::mt19937_64 random(workload_seed);
    std::uniform_int_distribution<std::size_t> pick_index(0, n - 1);

    std::vector<Key> live;
    live.reserve(n);
    std::unordered_set<Key> present;
    present.reserve(n + 1);

    for (std::size_t i = 0; i < n; ++i) {
        const Key key = fresh_key(random, present);
        live.push_back(key);

        if (!successor.insert(key) || !mixed.insert(key)) {
            throw std::runtime_error("initial insertion failed");
        }
    }

    checkpoint(successor, mixed, live, 0);

    const auto pair_count = static_cast<std::uint64_t>(n) * n;
    const auto checkpoint_interval = pair_count / 100;

    for (std::uint64_t pair = 1; pair <= pair_count; ++pair) {
        Key& slot = live[pick_index(random)];
        const Key old_key = slot;
        const Key new_key = fresh_key(random, present);

        present.erase(old_key);
        slot = new_key;

        if (!successor.erase(old_key) || !mixed.erase(old_key) ||
            !successor.insert(new_key) || !mixed.insert(new_key) ||
            successor.size() != n || mixed.size() != n) {
            throw std::runtime_error(
                "churn update failed at pair " + std::to_string(pair)
            );
        }

        if (pair % checkpoint_interval == 0) {
            checkpoint(successor, mixed, live, pair);
        }
    }
}

}  

int main() {
    try {
        std::cout << "n,pairs,policy,height,average_depth\n";

        for (const std::size_t n : {1'000, 2'000, 4'000}) {
            verify_churn(n);
        }

        std::cerr << "Structure A full-scale verification passed "
                     "(21,000,000 pairs per policy)\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}