// CS253/MA253 Project 3 Analyst module — deterministic four-strategy churn replay.
// Does not replace or modify either studied tree implementation.
#include "hibbard_bst.h"
#include "scapegoat_tree.h"
#include "baseline.h"
#include "degradation.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
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

Options parse(int argc, char** argv) {
    Options x;
    if (argc == 1 || (argc == 2 && std::string(argv[1]) == "--quick")) return x;
    if (argc < 6 || argc > 8) {
        throw std::invalid_argument("usage: churn_benchmark [--quick | n pairs seed trial workload [checkpoint [strategy]]]; workload = random|exact|shallow_exact|shallow_fresh");
    }
    x.n = std::stoi(argv[1]);
    x.pairs = std::stoull(argv[2]);
    x.seed = std::stoull(argv[3]);
    x.trial = std::stoi(argv[4]);
    x.mode = argv[5];
    x.checkpoint = argc >= 7 ? std::stoull(argv[6]) : static_cast<std::uint64_t>(x.n);
    if (argc == 8) x.strategy = argv[7];
    if (x.strategy != "all" && x.strategy != "successor" && x.strategy != "random_policy" &&
        x.strategy != "scapegoat_array" && x.strategy != "scapegoat_dsw")
        throw std::invalid_argument("unknown strategy");
    if (x.n <= 0 || x.n > 1000000 || x.pairs == 0 || x.checkpoint == 0 || x.trial <= 0)
        throw std::invalid_argument("n, pairs, checkpoint and trial must be positive (n <= 1000000)");
    if (x.mode != "random" && x.mode != "exact" &&
        x.mode != "shallow_exact" && x.mode != "shallow_fresh")
        throw std::invalid_argument("unknown workload mode");
    // Keep trace allocation bounded and avoid overflowing vector size calculations.
    if (x.pairs > 20000000ULL) throw std::invalid_argument("maximum 20 million pairs per invocation");
    return x;
}

struct Trace {
    std::vector<Key> initial;
    std::vector<Pair> pairs;
    std::uint64_t hash = 0;
};

Trace make_trace(const Options& o) {
    Trace t;
    t.initial = adversary::initialKeys(o.n, o.seed);
    t.pairs.reserve(static_cast<std::size_t>(o.pairs));
    adversary::TraceHash h;
    std::vector<unsigned char> live(static_cast<std::size_t>(16 * o.n), 0);
    for (Key key : t.initial) {
        if (key < 0 || key >= 16LL * o.n || live[static_cast<std::size_t>(key)])
            throw std::logic_error("invalid initial population");
        live[static_cast<std::size_t>(key)] = 1;
        h.add('I', key);
    }
    auto record = [&](Pair p) {
        if (p.removed < 0 || p.inserted < 0 ||
            p.removed >= 16LL * o.n || p.inserted >= 16LL * o.n ||
            !live[static_cast<std::size_t>(p.removed)])
            throw std::logic_error("generator deleted an absent key");
        live[static_cast<std::size_t>(p.removed)] = 0;
        if (live[static_cast<std::size_t>(p.inserted)])
            throw std::logic_error("generator inserted a live key");
        live[static_cast<std::size_t>(p.inserted)] = 1;
        h.add('D', p.removed);
        h.add('I', p.inserted);
        t.pairs.push_back(p);
    };
    if (o.mode == "random" || o.mode == "exact") {
        adversary::Baseline generator(o.n, o.seed, o.mode);
        for (std::uint64_t i = 0; i < o.pairs; ++i) record(generator.next());
    } else {
        adversary::Degradation generator(o.n, o.seed, o.mode);
        for (std::uint64_t i = 0; i < o.pairs; ++i) record(generator.next());
    }
    t.hash = h.get();
    return t;
}

struct Measured {
    std::string name;
    churn::HibbardBst* bst = nullptr;
    ScapegoatTree* scapegoat = nullptr;
    std::uint64_t duration_ns = 0;

    void insert(Key key) {
        if (bst) { if (!bst->insert(key)) throw std::logic_error("unexpected BST duplicate"); }
        else { if (scapegoat->contains(key)) throw std::logic_error("unexpected scapegoat duplicate"); scapegoat->insert(key); }
    }
    void apply(Pair pair) {
        if (bst) {
            if (!bst->erase(pair.removed) || !bst->insert(pair.inserted))
                throw std::logic_error("BST replay error");
        } else {
            if (!scapegoat->erase(pair.removed)) throw std::logic_error("scapegoat erase error");
            scapegoat->insert(pair.inserted); // returns void in teammate interface
        }
    }
    std::size_t size() const {
        return bst ? bst->size() : static_cast<std::size_t>(scapegoat->size());
    }
    double depth() const {return bst ? bst->metrics().average_depth() : scapegoat->averageDepth();}
    long long height() const {return bst ? static_cast<long long>(bst->metrics().height) : scapegoat->height();}
    std::vector<Key> ordered() const {return bst ? bst->inorder() : scapegoat->toVector();}
    bool valid() const {return bst ? bst->validate() : scapegoat->checkInvariants();}
    RebuildStats rebuilds() const {return scapegoat ? scapegoat->getRebuildStats() : RebuildStats{};}
};

std::vector<Key> expected_at(const Trace& t, std::size_t pair_limit, int n) {
    std::vector<unsigned char> live(static_cast<std::size_t>(16 * n), 0);
    for (Key k : t.initial) live[static_cast<std::size_t>(k)] = 1;
    for (std::size_t i = 0; i < pair_limit; ++i) {
        auto p = t.pairs[i];
        live[static_cast<std::size_t>(p.removed)] = 0;
        live[static_cast<std::size_t>(p.inserted)] = 1;
    }
    std::vector<Key> expected;
    expected.reserve(n);
    for (std::size_t k = 0; k < live.size(); ++k) if (live[k]) expected.push_back(static_cast<Key>(k));
    return expected;
}

void replay(const Options& o, const Trace& t, Measured& tree) {
    // Construction and trace generation are outside timing.
    for (Key k : t.initial) tree.insert(k);
    if (tree.size() != static_cast<std::size_t>(o.n) || !tree.valid())
        throw std::logic_error("initial tree invalid");
    auto expected = expected_at(t, 0, o.n);
    if (tree.ordered() != expected) throw std::logic_error("initial contents mismatch");

    if (tree.scapegoat) tree.scapegoat->resetRebuildStats();
    // Update oracle only at checkpoints, outside structure timing.
    std::vector<unsigned char> live(static_cast<std::size_t>(16 * o.n), 0);
    for (auto key : t.initial) live[static_cast<std::size_t>(key)] = 1;
    std::uint64_t start_pair = 0;
    while (start_pair < o.pairs) {
        const auto end_pair = std::min(o.pairs, start_pair + std::min(o.checkpoint, o.pairs - start_pair));
        const auto start = Clock::now();
        for (auto i = start_pair; i < end_pair; ++i) tree.apply(t.pairs[static_cast<std::size_t>(i)]);
        const auto end = Clock::now();
        tree.duration_ns += static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());

        for (auto i = start_pair; i < end_pair; ++i) {
            auto p = t.pairs[static_cast<std::size_t>(i)];
            if (!live[static_cast<std::size_t>(p.removed)])
                throw std::logic_error("oracle removed absent key");
            live[static_cast<std::size_t>(p.removed)] = 0;
            if (live[static_cast<std::size_t>(p.inserted)])
                throw std::logic_error("oracle inserted duplicate key");
            live[static_cast<std::size_t>(p.inserted)] = 1;
        }
        expected.clear();
        for (std::size_t k = 0; k < live.size(); ++k) if (live[k]) expected.push_back(static_cast<Key>(k));
        if (expected.size() != static_cast<std::size_t>(o.n) ||
            tree.size() != static_cast<std::size_t>(o.n) ||
            !tree.valid() || tree.ordered() != expected)
            throw std::logic_error("checkpoint invariant/contents mismatch");

        const RebuildStats r = tree.rebuilds();
        // ns_per_operation is for each delete or insert, hence denominator 2*pairs.
        const double ns_per_operation = static_cast<double>(tree.duration_ns) /
                                        (2.0 * static_cast<double>(end_pair));
        std::cout << o.mode << ',' << o.n << ',' << o.trial << ',' << o.seed << ','
                  << tree.name << ',' << end_pair << ',' << t.hash << ','
                  << std::setprecision(10) << tree.depth() << ',' << tree.height() << ','
                  << std::fixed << std::setprecision(3) << ns_per_operation << ','
                  << r.scapegoat_rebuilds << ',' << r.whole_tree_rebuilds << ','
                  << r.nodes_rebuilt << ',' << r.stout_warren_rotations << '\n';
        start_pair = end_pair;
    }
}
} // namespace

int main(int argc, char** argv) {
    try {
        const auto o = parse(argc, argv);
        const Trace trace = make_trace(o);
        std::cout << "workload,n,trial,seed,strategy,pairs,trace_hash,average_depth,height,ns_per_operation,local_rebuilds,whole_tree_rebuilds,nodes_rebuilt,dsw_rotations\n";
        const std::uint64_t policy_seed = o.seed ^ UINT64_C(0x504f4c494359);
        churn::HibbardBst successor(churn::DeletionPolicy::AlwaysSuccessor, policy_seed);
        churn::HibbardBst randomized(churn::DeletionPolicy::RandomPredecessorOrSuccessor, policy_seed);
        ScapegoatTree array(0.7, ARRAY_REBUILD);
        ScapegoatTree dsw(0.7, STOUT_WARREN_REBUILD);
        Measured trees[] = {
            {"successor", &successor, nullptr, 0},
            {"random_policy", &randomized, nullptr, 0},
            {"scapegoat_array", nullptr, &array, 0},
            {"scapegoat_dsw", nullptr, &dsw, 0}
        };
        // Rotate run order across trials to reduce systematic cache/warm-up bias.
        std::array<int, 4> run_order{{0, 1, 2, 3}};
        std::rotate(run_order.begin(), run_order.begin() + ((o.trial - 1) % 4), run_order.end());
        for (int index : run_order) {
            if (o.strategy == "all" || trees[index].name == o.strategy)
                replay(o, trace, trees[index]);
        }
    } catch (const std::exception& e) {
        std::cerr << "Benchmark error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
