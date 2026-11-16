#pragma once
#include "sorted_array.h"
#include "../src/structA/hibbard_bst.h"
#include "../src/structB/scapegoat_tree.h"
#include <cstdint>
#include <stdexcept>
#include <string>

namespace oracle {
struct Op { char kind; Key key; };
constexpr std::uint64_t policy_seed = 253;
constexpr std::size_t audit_interval = 4096;
constexpr double alpha = 0.7;

struct Failure : std::runtime_error {
    std::string variant, check;
    Failure(const char* name, const char* rule)
        : std::runtime_error(std::string(name) + "/" + rule), variant(name), check(rule) {}
};

inline void require(bool ok, const char* variant, const char* rule) {
    if (!ok) throw Failure(variant, rule);
}

class Checks {
    SortedArray ref_;
    churn::HibbardBst successor_;
    churn::HibbardBst random_{churn::DeletionPolicy::RandomPredecessorOrSuccessor, policy_seed};
    ScapegoatTree array_{alpha, ARRAY_REBUILD};
    ScapegoatTree vine_{alpha, STOUT_WARREN_REBUILD};

    void compare(churn::HibbardBst& tree, const char* name, Op op, bool expected) {
        if (op.kind == 'I') require(tree.insert(op.key) == expected, name, "insert-return");
        if (op.kind == 'D') require(tree.erase(op.key) == expected, name, "erase-return");
        require(tree.size() == ref_.size(), name, "size");
        require(tree.contains(op.key) == ref_.contains(op.key), name, "membership");
    }

    void compare(ScapegoatTree& tree, const char* name, Op op, bool expected) {
        if (op.kind == 'I') tree.insert(op.key); // B's insert returns void on main.
        if (op.kind == 'D') require(tree.erase(op.key) == expected, name, "erase-return");
        require(static_cast<std::size_t>(tree.size()) == ref_.size(), name, "size");
        require(tree.contains(op.key) == ref_.contains(op.key), name, "membership");
    }

    void audit(const churn::HibbardBst& tree, const char* name) const {
        require(tree.validate(), name, "order-and-count");
        require(tree.inorder() == ref_.keys(), name, "inorder");
    }

    void audit(const ScapegoatTree& tree, const char* name) const {
        require(tree.checkInvariants(), name, "scapegoat-invariants");
        require(tree.toVector() == ref_.keys(), name, "inorder");
        require(tree.size() >= alpha * tree.getMaxN(), name, "shrink-threshold");
    }

public:
    void apply(Op op) {
        bool expected;
        switch (op.kind) {
        case 'I': expected = ref_.insert(op.key); break;
        case 'D': expected = ref_.erase(op.key); break;
        case 'C': expected = ref_.contains(op.key); break;
        default: throw std::invalid_argument("unknown operation");
        }
        compare(successor_, "A-successor", op, expected);
        compare(random_, "A-random", op, expected);
        compare(array_, "B-array", op, expected);
        compare(vine_, "B-Stout-Warren", op, expected);
    }

    void audit() const {
        audit(successor_, "A-successor");
        audit(random_, "A-random");
        audit(array_, "B-array");
        audit(vine_, "B-Stout-Warren");
    }
};
}
