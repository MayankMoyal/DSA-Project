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

int main() {
    try {
        test_insert_and_lookup();
        std::cout << "Structure A focused checks passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}