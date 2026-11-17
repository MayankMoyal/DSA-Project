// Analyst baseline: insertion and successful lookup on the current Hibbard BST.
// This is NOT the required long-running insert/delete churn experiment.
#include "hibbard_bst.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

using Clock = std::chrono::steady_clock;

static double milliseconds(Clock::time_point start, Clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - start).count();
}

int main() {
    const int sizes[] = {1000, 2000, 4000};
    const int trials = 5;

    std::cout << "mode,n,trial,seed,insert_ms,lookup_ms,lookups\n";

    for (int n : sizes) {
        for (int mode = 0; mode < 2; ++mode) {
            for (int trial = 1; trial <= trials; ++trial) {
                // Fixed, printed seed: runs can be reproduced on any machine.
                const std::uint32_t seed =
                    20261114u + static_cast<std::uint32_t>(n * 31 + trial);

                std::vector<long long> keys(n);
                std::iota(keys.begin(), keys.end(), 0LL);

                if (mode == 1) {
                    std::mt19937 insertion_rng(seed);
                    std::shuffle(keys.begin(), keys.end(), insertion_rng);
                }

                // Lookups use a separate reproducible shuffled order.
                std::vector<long long> queries(keys);
                std::mt19937 lookup_rng(seed ^ 0x9e3779b9u);
                std::shuffle(queries.begin(), queries.end(), lookup_rng);

                churn::HibbardBst tree;

                const auto insert_start = Clock::now();
                for (long long key : keys) {
                    if (!tree.insert(key)) {
                        throw std::runtime_error("unexpected duplicate key");
                    }
                }
                const auto insert_end = Clock::now();

                std::size_t found = 0;
                const auto lookup_start = Clock::now();
                for (long long key : queries) {
                    found += tree.contains(key) ? 1u : 0u;
                }
                const auto lookup_end = Clock::now();

                if (found != static_cast<std::size_t>(n) ||
                    tree.size() != static_cast<std::size_t>(n)) {
                    throw std::runtime_error("BST sanity check failed");
                }

                std::cout << (mode == 0 ? "sorted" : "shuffled") << ','
                          << n << ',' << trial << ',' << seed << ','
                          << milliseconds(insert_start, insert_end) << ','
                          << milliseconds(lookup_start, lookup_end) << ','
                          << found << '\n';
            }
        }
    }
}
