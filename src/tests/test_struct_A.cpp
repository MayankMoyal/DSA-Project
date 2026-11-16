#include "hibbard_bst.h"

#include <initializer_list>
#include <iostream>
#include <stdexcept>

using churn::HibbardBst;
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
int main() {
    try {
        test_insert_and_lookup();
        test_successor_deletion();
        std::cout << "Structure A focused checks passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}