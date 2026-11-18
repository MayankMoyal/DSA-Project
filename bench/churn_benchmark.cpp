// Analyst benchmark: the same delete/insert trace is replayed on four trees.
// Existing tree implementations and the adversary generators are unchanged.
#include "hibbard_bst.h"
#include "scapegoat_tree.h"
#include "baseline.h"
#include "degradation.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using Clock = std::chrono::steady_clock;
using Key = long long;
using Pair = adversary::Pair;

struct Options {
    int n = 100;
    std::uint64_t pairs = 1000;
    std::uint64_t seed = 20261116;
    int trial = 1;
    std::string mode = "random";
    std::string strategy = "all";
    std::uint64_t checkpoint = 100;
};

Options parseOptions(int argc, char** argv) {
    Options options;
    if (argc == 1) {
        return options;
    }
    if (argc == 2 && std::string(argv[1]) == "--quick") {
        return options;
    }
    if (argc < 6 || argc > 8) {
        throw std::invalid_argument("usage: churn_benchmark [--quick | n pairs seed trial workload [checkpoint [strategy]]]; workload = random|exact|shallow_exact|shallow_fresh");
    }
    options.n = std::stoi(argv[1]);
    options.pairs = std::stoull(argv[2]);
    options.seed = std::stoull(argv[3]);
    options.trial = std::stoi(argv[4]);
    options.mode = argv[5];
    if (argc >= 7) {
        options.checkpoint = std::stoull(argv[6]);
    } else {
        options.checkpoint = static_cast<std::uint64_t>(options.n);
    }
    if (argc == 8) {
        options.strategy = argv[7];
    }
    if (options.strategy != "all" && options.strategy != "successor" &&
        options.strategy != "random_policy" && options.strategy != "scapegoat_array" &&
        options.strategy != "scapegoat_dsw") {
        throw std::invalid_argument("unknown strategy");
    }
    if (options.n <= 0 || options.n > 1000000 || options.pairs == 0 ||
        options.checkpoint == 0 || options.trial <= 0) {
        throw std::invalid_argument("n, pairs, checkpoint and trial must be positive (n <= 1000000)");
    }
    if (options.mode != "random" && options.mode != "exact" &&
        options.mode != "shallow_exact" && options.mode != "shallow_fresh") {
        throw std::invalid_argument("unknown workload mode");
    }
    if (options.pairs > 20000000ULL) {
        throw std::invalid_argument("maximum 20 million pairs per invocation");
    }
    return options;
}

struct Trace {
    std::vector<Key> initial;
    std::vector<Pair> pairs;
    std::uint64_t hash = 0;
};

// A membership table: live[key] is 1 if the key is present and 0 otherwise.
// Used for correctness checking, not as the tree being measured.
void updateMembership(std::vector<unsigned char>& live, Pair pair) {
    Key removed = pair.removed;
    Key inserted = pair.inserted;
    Key limit = static_cast<Key>(live.size());
    if (removed < 0 || removed >= limit || inserted < 0 || inserted >= limit) {
        throw std::logic_error("key outside universe");
    }
    if (live[removed] == 0) {
        throw std::logic_error("deleted an absent key");
    }
    live[removed] = 0;
    if (live[inserted] != 0) {
        throw std::logic_error("inserted a live key");
    }
    live[inserted] = 1;
}

void recordPair(Trace& trace, std::vector<unsigned char>& live,
                adversary::TraceHash& hash, Pair pair) {
    updateMembership(live, pair);
    hash.add('D', pair.removed);
    hash.add('I', pair.inserted);
    trace.pairs.push_back(pair);
}

Trace makeTrace(const Options& options) {
    Trace trace;
    trace.initial = adversary::initialKeys(options.n, options.seed);
    trace.pairs.reserve(static_cast<std::size_t>(options.pairs));
    adversary::TraceHash hash;
    std::vector<unsigned char> live(16 * options.n, 0);

    for (Key key : trace.initial) {
        if (key < 0 || key >= 16LL * options.n || live[key] != 0) {
            throw std::logic_error("invalid initial population");
        }
        live[key] = 1;
        hash.add('I', key);
    }
    // Reuse Mayank's existing generator, with the same seed and workload.
    if (options.mode == "random" || options.mode == "exact") {
        adversary::Baseline generator(options.n, options.seed, options.mode);
        for (std::uint64_t i = 0; i < options.pairs; i++) {
            Pair pair = generator.next();
            recordPair(trace, live, hash, pair);
        }
    } else {
        adversary::Degradation generator(options.n, options.seed, options.mode);
        for (std::uint64_t i = 0; i < options.pairs; i++) {
            Pair pair = generator.next();
            recordPair(trace, live, hash, pair);
        }
    }
    trace.hash = hash.get();
    return trace;
}

// The two tree classes have different APIs. This small wrapper gives the
// benchmark one common interface. Exactly one pointer is non-null.
struct MeasuredTree {
    std::string name;
    churn::HibbardBst* bst = nullptr;
    ScapegoatTree* scapegoat = nullptr;
    std::uint64_t duration_ns = 0;

    void insertInitial(Key key) {
        if (bst != nullptr) {
            bool inserted = bst->insert(key);
            if (!inserted) {
                throw std::logic_error("unexpected BST duplicate");
            }
        } else {
            if (scapegoat->contains(key)) {
                throw std::logic_error("unexpected scapegoat duplicate");
            }
            scapegoat->insert(key);
        }
    }

    void apply(Pair pair) {
        if (bst != nullptr) {
            bool deleted = bst->erase(pair.removed);
            if (deleted == false) {
                throw std::logic_error("BST deletion failed");
            }
            bool inserted = bst->insert(pair.inserted);
            if (inserted == false) {
                throw std::logic_error("BST insertion failed");
            }
        } else {
            bool deleted = scapegoat->erase(pair.removed);
            if (deleted == false) {
                throw std::logic_error("Scapegoat deletion failed");
            }
            // Scapegoat insert returns void. Check contents at checkpoints.
            scapegoat->insert(pair.inserted);
        }
    }

    std::size_t size() const {
        if (bst != nullptr) {
            return bst->size();
        }
        return static_cast<std::size_t>(scapegoat->size());
    }

    double averageDepth() const {
        if (bst != nullptr) {
            return bst->metrics().average_depth();
        }
        return scapegoat->averageDepth();
    }

    long long height() const {
        if (bst != nullptr) {
            return static_cast<long long>(bst->metrics().height);
        }
        return scapegoat->height();
    }

    std::vector<Key> orderedKeys() const {
        if (bst != nullptr) {
            return bst->inorder();
        }
        return scapegoat->toVector();
    }

    bool valid() const {
        if (bst != nullptr) {
            return bst->validate();
        }
        return scapegoat->checkInvariants();
    }

    RebuildStats rebuilds() const {
        if (scapegoat != nullptr) {
            return scapegoat->getRebuildStats();
        }
        return RebuildStats{};
    }
};

// Scanning the membership table in ascending order produces the expected
// inorder traversal. It contains every live key exactly once.
std::vector<Key> expectedKeys(const std::vector<unsigned char>& live, int n) {
    std::vector<Key> expected;
    expected.reserve(n);
    for (std::size_t key = 0; key < live.size(); key++) {
        if (live[key] != 0) {
            expected.push_back(static_cast<Key>(key));
        }
    }
    return expected;
}

void checkTree(const MeasuredTree& tree,
               const std::vector<unsigned char>& live, int n) {
    std::vector<Key> expected = expectedKeys(live, n);
    if (expected.size() != static_cast<std::size_t>(n) ||
        tree.size() != static_cast<std::size_t>(n) || !tree.valid() ||
        tree.orderedKeys() != expected) {
        throw std::logic_error("checkpoint invariant/contents mismatch");
    }
}

// Only the tree updates are inside the timed interval.
void measureUpdates(MeasuredTree& tree, const Trace& trace,
                    std::uint64_t startPair, std::uint64_t endPair) {
    Clock::time_point start = Clock::now();
    for (std::uint64_t i = startPair; i < endPair; i++) {
        tree.apply(trace.pairs[i]);
    }
    Clock::time_point end = Clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    tree.duration_ns += static_cast<std::uint64_t>(elapsed.count());
}

void printCheckpoint(const Options& options, const Trace& trace,
                     const MeasuredTree& tree, std::uint64_t completedPairs) {
    RebuildStats stats = tree.rebuilds();
    // Every pair is two operations: one deletion and one insertion.
    double operations = 2.0 * static_cast<double>(completedPairs);
    double nsPerOperation = static_cast<double>(tree.duration_ns) / operations;
    std::cout << options.mode << ',' << options.n << ',' << options.trial << ','
              << options.seed << ',' << tree.name << ',' << completedPairs << ','
              << trace.hash << ',' << std::setprecision(10) << tree.averageDepth() << ','
              << tree.height() << ',' << std::fixed << std::setprecision(3)
              << nsPerOperation << ',' << stats.scapegoat_rebuilds << ','
              << stats.whole_tree_rebuilds << ',' << stats.nodes_rebuilt << ','
              << stats.stout_warren_rotations << '\n';
}

void replay(const Options& options, const Trace& trace, MeasuredTree& tree) {
    std::vector<unsigned char> live(16 * options.n, 0);
    for (Key key : trace.initial) {
        tree.insertInitial(key);
        live[key] = 1;
    }
    checkTree(tree, live, options.n);
    // Do not count rebuilds during initial construction as churn rebuilds.
    if (tree.scapegoat != nullptr) {
        tree.scapegoat->resetRebuildStats();
    }

    std::uint64_t startPair = 0;
    while (startPair < options.pairs) {
        std::uint64_t remaining = options.pairs - startPair;
        std::uint64_t blockSize = std::min(options.checkpoint, remaining);
        std::uint64_t endPair = startPair + blockSize;

        measureUpdates(tree, trace, startPair, endPair);

        // Oracle, validation and printing stay OUTSIDE the timed interval.
        for (std::uint64_t i = startPair; i < endPair; i++) {
            updateMembership(live, trace.pairs[i]);
        }
        checkTree(tree, live, options.n);
        printCheckpoint(options, trace, tree, endPair);
        startPair = endPair;
    }
}

void runAllStrategies(const Options& options, const Trace& trace) {
    // Separate random stream for Hibbard replacement-side choices.
    std::uint64_t policySeed = options.seed ^ UINT64_C(0x504f4c494359);
    churn::HibbardBst successor(churn::DeletionPolicy::AlwaysSuccessor, policySeed);
    churn::HibbardBst randomized(churn::DeletionPolicy::RandomPredecessorOrSuccessor, policySeed);
    ScapegoatTree arrayTree(0.7, ARRAY_REBUILD);
    ScapegoatTree dswTree(0.7, STOUT_WARREN_REBUILD);
    MeasuredTree trees[] = {
        {"successor", &successor, nullptr, 0},
        {"random_policy", &randomized, nullptr, 0},
        {"scapegoat_array", nullptr, &arrayTree, 0},
        {"scapegoat_dsw", nullptr, &dswTree, 0}
    };
    // Rotate starting index each trial, just like the original code.
    int first = (options.trial - 1) % 4;
    for (int offset = 0; offset < 4; offset++) {
        int index = (first + offset) % 4;
        if (options.strategy == "all" || options.strategy == trees[index].name) {
            replay(options, trace, trees[index]);
        }
    }
}
} // namespace

int main(int argc, char** argv) {
    try {
        Options options = parseOptions(argc, argv);
        Trace trace = makeTrace(options);
        std::cout << "workload,n,trial,seed,strategy,pairs,trace_hash,average_depth,height,ns_per_operation,local_rebuilds,whole_tree_rebuilds,nodes_rebuilt,dsw_rotations\n";
        runAllStrategies(options, trace);
    } catch (const std::exception& error) {
        std::cerr << "Benchmark error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
