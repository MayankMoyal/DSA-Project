// Project 3 - Search trees under long-running churn
// Architect's application: reproducible fixed-size churn replay and depth samples.
// Build and run from the repository root. See README.md.

#include "include/tree_adapter.h"
#include "adversary/baseline.h"
#include "adversary/degradation.h"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using churn::Key;

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

class Strategies {
public:
    explicit Strategies(std::uint64_t seed)
        : randomized_(churn::DeletionPolicy::RandomPredecessorOrSuccessor, seed) {}

    template <typename Action>
    void each(Action action) {
        action(successor_, "hibbard_successor");
        action(randomized_, "hibbard_randomized");
        action(array_, "scapegoat_array");
        action(stout_warren_, "scapegoat_stout_warren");
    }

private:
    churn::HibbardBst successor_;
    churn::HibbardBst randomized_;
    ScapegoatTree array_{0.7, ARRAY_REBUILD};
    ScapegoatTree stout_warren_{0.7, STOUT_WARREN_REBUILD};
};

[[noreturn]] void replay_error(const char* strategy, const char* message,
                               std::uint64_t completed) {
    throw std::logic_error(std::string(strategy) + ": " + message + " at pair " +
                           std::to_string(completed));
}

void simulate(Generator& generator, int n, std::uint64_t pairs, std::uint64_t seed,
              const std::string& mode, std::uint64_t sample_every) {
    const auto population = static_cast<std::size_t>(n);
    const auto universe = adversary::universeSize(n);
    std::vector<unsigned char> live(universe, 0);
    adversary::TraceHash trace_hash;
    Strategies strategies(seed);

    for (Key key : adversary::initialKeys(n, seed)) {
        if (key < 0 || key >= universe || live[key])
            replay_error("generator", "invalid setup key", 0);
        strategies.each([&](auto& tree, const char* name) {
            if (!churn::add_key(tree, key))
                replay_error(name, "setup insertion failed", 0);
        });
        live[key] = 1;
        trace_hash.add('I', key);
    }
    strategies.each([](auto& tree, const char*) { churn::begin_churn(tree); });

    std::cout << "strategy,workload,n,pairs_done,seed,average_depth,height,live_size,"
                 "trace_hash,rebuild_events,nodes_rebuilt,stout_warren_rotations\n"
              << std::setprecision(12);
    auto sample = [&](std::uint64_t completed) {
        std::vector<Key> expected;
        expected.reserve(population);
        for (int key = 0; key < universe; ++key) {
            if (live[key]) expected.push_back(key);
        }
        if (expected.size() != population)
            replay_error("generator", "invalid live population", completed);

        strategies.each([&](const auto& tree, const char* name) {
            const auto stats = churn::inspect(tree);
            if (stats.count != population || static_cast<std::size_t>(tree.size()) != population ||
                !churn::correct_shape(tree) || churn::keys_in_order(tree) != expected)
                replay_error(name, "tree invariant or live-key set mismatch", completed);

            std::cout << name << ',' << mode << ',' << n << ',' << completed << ','
                      << seed << ',' << stats.average_depth << ',' << stats.height << ','
                      << stats.count << ',' << trace_hash.get() << ',' << stats.rebuild_events
                      << ',' << stats.nodes_rebuilt << ',' << stats.rotations << '\n';
        });
        std::cout.flush();
        if (!std::cout) throw std::runtime_error("could not write CSV");
    };

    sample(0);
    std::uint64_t completed = 0;
    while (completed < pairs) {
        const auto pair = generator.next();
        const auto removed = pair.removed;
        const auto inserted = pair.inserted;
        ++completed;
        if (removed < 0 || removed >= universe || !live[removed])
            replay_error("generator", "deletion must name a live key", completed);
        if (inserted < 0 || inserted >= universe || (inserted != removed && live[inserted]))
            replay_error("generator", "insertion must name an absent key after deletion", completed);

        strategies.each([&](auto& tree, const char* name) {
            if (!tree.erase(removed) || static_cast<std::size_t>(tree.size()) != population - 1)
                replay_error(name, "deletion failed or changed size incorrectly", completed);
            if (!churn::add_key(tree, inserted) || static_cast<std::size_t>(tree.size()) != population)
                replay_error(name, "insertion failed or changed size incorrectly", completed);
        });

        live[removed] = 0;
        live[inserted] = 1;
        trace_hash.add('D', removed);
        trace_hash.add('I', inserted);
        if (completed % sample_every == 0 || completed == pairs) sample(completed);
    }

    std::cout.flush();
    if (!std::cout) throw std::runtime_error("could not write CSV");
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

        Generator generator(n, seed, mode);
        simulate(generator, n, pairs, seed, mode, every);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Churn simulator error: " << error.what() << '\n';
        return 1;
    }
}
