#include "hibbard_bst.h"

#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <limits>
#include <vector>

using churn::HibbardBst;
using churn::DeletionPolicy;
using Key = HibbardBst::Key;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void add_keys(HibbardBst& tree, std::initializer_list<Key> keys) {
    for (Key key : keys) {
        require(tree.insert(key), "fixture insertion failed");
    }
}

void test_insert_and_lookup() {
    HibbardBst tree;
    require(tree.size() == 0 && !tree.contains(7), "empty tree");

    add_keys(tree, {10, 5, 20, 7});
    require(tree.contains(10) && tree.contains(7), "inserted key missing");
    require(!tree.contains(8), "absent key found");
    require(!tree.insert(5) && tree.size() == 4, "duplicate changed the tree");

    tree.clear();
    tree.clear();
    require(tree.size() == 0 && !tree.contains(10), "clear failed");
    require(tree.insert(10), "reuse after clear failed");
}
void test_successor_deletion() {
    HibbardBst tree;
    require(!tree.erase(99), "empty erase succeeded");
    add_keys(tree, {10, 5, 20, 15, 17, 3, 4});

    require(tree.erase(10) && !tree.contains(10), "root erase failed");
    require(tree.contains(15) && tree.contains(17), "deep successor lost a child");
    require(tree.erase(3) && tree.contains(4), "one-child splice failed");
    require(tree.erase(4) && !tree.contains(4), "leaf erase failed");
    require(!tree.erase(4) && tree.size() == 4, "missing erase changed size");

    HibbardBst immediate;
    add_keys(immediate, {10, 5, 15, 17});
    require(immediate.erase(10), "immediate successor erase failed");
    require(immediate.contains(15) && immediate.contains(17) &&
                immediate.size() == 3,
            "immediate successor lost a child");

    HibbardBst root;
    add_keys(root, {10, 5});
    require(root.erase(10) && root.contains(5), "one-child root splice failed");
    require(root.erase(5) && root.size() == 0, "singleton root erase failed");
}
void expect_keys(const HibbardBst& tree, std::initializer_list<Key> keys) {
    require(tree.validate(), "BST order or size invariant failed");
    require(tree.size() == keys.size(), "reported size differs");
    require(tree.inorder() == std::vector<Key>(keys), "inorder keys differ");
}

void test_order_and_bounds() {
    HibbardBst tree;
    expect_keys(tree, {});

    const Key low = std::numeric_limits<Key>::min();
    const Key high = std::numeric_limits<Key>::max();
    add_keys(tree, {0, high, low, -1, 1});
    expect_keys(tree, {low, -1, 0, 1, high});

    require(tree.erase(0), "interior key erase failed");
    expect_keys(tree, {low, -1, 1, high});
}
void test_metrics() {
    HibbardBst tree;
    const auto empty = tree.metrics();

    require(empty.size == 0 && empty.height == -1 &&
                empty.internal_path_length == 0 && empty.average_depth() == 0.0,
            "empty metric conventions");

    add_keys(tree, {10, 5, 20, 15, 17});
    const auto before = tree.metrics();

    require(before.size == 5 && before.height == 3 &&
                before.internal_path_length == 7 &&
                before.average_depth() == 7.0 / 5.0,
            "wrong depth or IPL before deletion");

    require(tree.erase(10), "measured root erase failed");
    const auto after = tree.metrics();

    require(after.size == 4 && after.height == 2 &&
                after.internal_path_length == 4 && after.average_depth() == 1.0,
            "wrong depth or IPL after deletion");
}
void test_replacement_policies() {
    HibbardBst fixed(DeletionPolicy::AlwaysSuccessor, 3);
    add_keys(fixed, {10, 5, 7, 6, 20});
    require(fixed.erase(10), "fixed-policy erase failed");
    expect_keys(fixed, {5, 6, 7, 20});
    require(fixed.metrics().height == 3, "fixed policy must choose successor");

    HibbardBst random_successor(DeletionPolicy::RandomPredecessorOrSuccessor, 0);
    add_keys(random_successor, {10, 5, 7, 6, 20});
    require(random_successor.erase(10), "random successor erase failed");
    expect_keys(random_successor, {5, 6, 7, 20});
    require(random_successor.metrics().height == 3, "seed 0 must choose successor first");

    HibbardBst deep_predecessor(DeletionPolicy::RandomPredecessorOrSuccessor, 3);
    add_keys(deep_predecessor, {10, 5, 7, 6, 20});
    require(deep_predecessor.erase(10), "deep predecessor erase failed");
    expect_keys(deep_predecessor, {5, 6, 7, 20});
    require(deep_predecessor.metrics().height == 2, "seed 3 must choose predecessor first");

    HibbardBst immediate_predecessor(DeletionPolicy::RandomPredecessorOrSuccessor, 3);
    add_keys(immediate_predecessor, {10, 5, 3, 20});
    require(immediate_predecessor.erase(10), "immediate predecessor erase failed");
    expect_keys(immediate_predecessor, {3, 5, 20});
    require(immediate_predecessor.metrics().height == 1,
            "immediate predecessor splice failed");
}
int main() {
    try {
        test_insert_and_lookup();
        test_successor_deletion();
        test_order_and_bounds();
        test_metrics();
        test_replacement_policies();
        std::cout << "Structure A focused checks passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}