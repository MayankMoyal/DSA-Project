// Project 3 - Search trees under long-running churn
// Architect's application: reproducible fixed-size churn replay and depth samples.
// Build from the repository root. See command in the accompanying response.

#include "src/structA/hibbard_bst.h"
#include "src/structB/scapegoat_tree.h"
#include "adversary/baseline.h"
#include "adversary/degradation.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using Key = long long;

struct Snapshot {
    std::size_t count;
    long long height;
    double average_depth;
    long long rebuild_events;
    long long nodes_rebuilt;
    long long rotations;
};

Snapshot inspect(const churn::HibbardBst& tree) {
    const auto m = tree.metrics();
    return {m.size, static_cast<long long>(m.height), m.average_depth(), 0, 0, 0};
}

Snapshot inspect(const ScapegoatTree& tree) {
    const auto depth = tree.depthStats();
    const auto& rebuild = tree.getRebuildStats();
    return {static_cast<std::size_t>(depth.count), depth.height, depth.average(),
            rebuild.scapegoat_rebuilds + rebuild.whole_tree_rebuilds,
            rebuild.nodes_rebuilt, rebuild.stout_warren_rotations};
}

bool correct_shape(const churn::HibbardBst& tree) { return tree.validate(); }
bool correct_shape(const ScapegoatTree& tree) { return tree.checkInvariants(); }

// Exclude initialization rebuilds from churn-only counters.
void begin_churn(churn::HibbardBst&) {}
void begin_churn(ScapegoatTree& tree) { tree.resetRebuildStats(); }

std::vector<Key> keys_in_order(const churn::HibbardBst& tree) {
    return tree.inorder();
}
std::vector<Key> keys_in_order(const ScapegoatTree& tree) {
    return tree.toVector();
}

bool add_key(churn::HibbardBst& tree, Key key) {
    return tree.insert(key);
}
bool add_key(ScapegoatTree& tree, Key key) {
    // ScapegoatTree::insert returns void; check the precondition explicitly.
    if (tree.contains(key)) return false;
    tree.insert(key);
    return true;
}

class Generator {
public:
    Generator(int n, std::uint64_t seed, const std::string& mode) {
        if (mode == "random" || mode == "exact") {
            baseline_ = std::make_unique<adversary::Baseline>(n, seed, mode);
        } else if (mode == "shallow_exact" || mode == "shallow_fresh") {
            degradation_ = std::make_unique<adversary::Degradation>(n, seed, mode);
        } else {
            throw std::invalid_argument("mode must be random, exact, shallow_exact, or shallow_fresh");
        }
    }

    adversary::Pair next() {
        return baseline_ ? baseline_->next() : degradation_->next();
    }

private:
    std::unique_ptr<adversary::Baseline> baseline_;
    std::unique_ptr<adversary::Degradation> degradation_;
};

std::uint64_t parse_number(const char* value, const char* name) {
    if (!value || !*value) throw std::invalid_argument(std::string("missing ") + name);
    for (const char* p = value; *p; ++p) {
        if (*p < '0' || *p > '9')
            throw std::invalid_argument(std::string(name) + " must be a nonnegative integer");
    }
    std::size_t consumed = 0;
    const auto result = std::stoull(value, &consumed, 10);
    if (consumed != std::string(value).size())
        throw std::invalid_argument(std::string("invalid ") + name);
    return static_cast<std::uint64_t>(result);
}

void verify_live_set(const std::vector<unsigned char>& live,
                     const std::vector<Key>& actual,
                     std::size_t n, const std::string& name,
                     std::uint64_t completed) {
    std::vector<Key> expected;
    expected.reserve(n);
    for (std::size_t k = 0; k < live.size(); ++k)
        if (live[k]) expected.push_back(static_cast<Key>(k));
    if (expected.size() != n || actual != expected)
        throw std::logic_error(name + ": wrong live-key set at pair " +
                               std::to_string(completed));
}

template <typename Tree>
std::uint64_t run_strategy(Tree& tree, const std::string& name,
                           int n, std::uint64_t pairs, std::uint64_t seed,
                           const std::string& mode, std::uint64_t sample_every) {
    const auto initial = adversary::initialKeys(n, seed);
    Generator generator(n, seed, mode); // Recreated with the SAME seed for each tree.
    std::vector<unsigned char> live(static_cast<std::size_t>(adversary::universeSize(n)), 0);
    adversary::TraceHash trace_hash;

    for (Key key : initial) {
        if (key < 0 || static_cast<std::size_t>(key) >= live.size() ||
            live[static_cast<std::size_t>(key)] || !add_key(tree, key))
            throw std::logic_error(name + ": failed to build initial tree");
        live[static_cast<std::size_t>(key)] = 1;
        trace_hash.add('I', key);
    }

    auto sample = [&](std::uint64_t completed) {
        const Snapshot s = inspect(tree);
        if (s.count != static_cast<std::size_t>(n) ||
            static_cast<std::size_t>(tree.size()) != static_cast<std::size_t>(n) ||
            !correct_shape(tree))
            throw std::logic_error(name + ": tree invariant/size failure at pair " +
                                   std::to_string(completed));
        verify_live_set(live, keys_in_order(tree), static_cast<std::size_t>(n), name, completed);
        std::cout << name << ',' << mode << ',' << n << ',' << completed
                  << ',' << seed << ',' << std::setprecision(12) << s.average_depth
                  << ',' << s.height << ',' << s.count << ',' << trace_hash.get()
                  << ',' << s.rebuild_events << ',' << s.nodes_rebuilt
                  << ',' << s.rotations << '\n';
    };

    begin_churn(tree);
    sample(0);
    for (std::uint64_t i = 1; i <= pairs; ++i) {
        const adversary::Pair p = generator.next();
        if (p.removed < 0 || p.inserted < 0 ||
            static_cast<std::size_t>(p.removed) >= live.size() ||
            static_cast<std::size_t>(p.inserted) >= live.size() ||
            !live[static_cast<std::size_t>(p.removed)])
            throw std::logic_error(name + ": invalid generated deletion at pair " + std::to_string(i));

        if (!tree.erase(p.removed))
            throw std::logic_error(name + ": deletion failed at pair " + std::to_string(i));
        live[static_cast<std::size_t>(p.removed)] = 0;
        trace_hash.add('D', p.removed);

        if (live[static_cast<std::size_t>(p.inserted)] || !add_key(tree, p.inserted))
            throw std::logic_error(name + ": duplicate/failed insertion at pair " + std::to_string(i));
        live[static_cast<std::size_t>(p.inserted)] = 1;
        trace_hash.add('I', p.inserted);

        if (static_cast<std::size_t>(tree.size()) != static_cast<std::size_t>(n))
            throw std::logic_error(name + ": size changed at pair " + std::to_string(i));

        if (i % sample_every == 0 || i == pairs) sample(i);
    }
    return trace_hash.get();
}

void print_usage(const char* program) {
    std::cerr << "Usage: " << program
              << " <n> <pairs> <seed> [random|exact|shallow_exact|shallow_fresh] [sample_every]\n"
              << "Runs all four strategies and prints depth CSV to stdout.\n"
              << "Example: " << program << " 1000 1000000 42 random 10000 > results.csv\n";
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 4 || argc > 6) {
        print_usage(argv[0]);
        return 2;
    }
    try {
        const auto n_input = parse_number(argv[1], "n");
        const auto pairs = parse_number(argv[2], "pairs");
        const auto seed = parse_number(argv[3], "seed");
        if (n_input == 0 || n_input > 1000000)
            throw std::invalid_argument("n must be in [1, 1000000]");
        const int n = static_cast<int>(n_input);
        const std::string mode = argc >= 5 ? argv[4] : "random";
        const std::uint64_t every = argc == 6
            ? parse_number(argv[5], "sample_every")
            : std::max<std::uint64_t>(1, pairs / 100);
        if (every == 0) throw std::invalid_argument("sample_every must be positive");

        std::cout << "strategy,workload,n,pairs_done,seed,average_depth,height,live_size,"
                     "trace_hash,rebuild_events,nodes_rebuilt,stout_warren_rotations\n";
        std::uint64_t expected_hash = 0;
        bool first = true;
        auto confirm = [&](std::uint64_t hash) {
            if (first) { expected_hash = hash; first = false; }
            else if (hash != expected_hash)
                throw std::logic_error("different operation traces across strategies");
        };
        {
            churn::HibbardBst tree(churn::DeletionPolicy::AlwaysSuccessor, seed);
            confirm(run_strategy(tree, "hibbard_successor", n, pairs, seed, mode, every));
        }
        {
            churn::HibbardBst tree(churn::DeletionPolicy::RandomPredecessorOrSuccessor, seed);
            confirm(run_strategy(tree, "hibbard_randomized", n, pairs, seed, mode, every));
        }
        {
            ScapegoatTree tree(0.7, ARRAY_REBUILD);
            confirm(run_strategy(tree, "scapegoat_array", n, pairs, seed, mode, every));
        }
        {
            ScapegoatTree tree(0.7, STOUT_WARREN_REBUILD);
            confirm(run_strategy(tree, "scapegoat_stout_warren", n, pairs, seed, mode, every));
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Churn simulator error: " << error.what() << '\n';
        return 1;
    }
}
