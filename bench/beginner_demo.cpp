// Optional learning file: three tiny churn pairs on the real four trees.
// No random generator, timer, CSV or statistics: just see the operations.
// This demo is NOT a replacement for the submitted benchmark.
#include "hibbard_bst.h"
#include "scapegoat_tree.h"
#include <iostream>
#include <vector>

void printKeys(const std::vector<long long>& keys) {
    for (long long key : keys) {
        std::cout << key << ' ';
    }
    std::cout << '\n';
}

int main() {
    churn::HibbardBst successor(churn::DeletionPolicy::AlwaysSuccessor, 42);
    churn::HibbardBst randomized(churn::DeletionPolicy::RandomPredecessorOrSuccessor, 42);
    ScapegoatTree arrayTree(0.7, ARRAY_REBUILD);
    ScapegoatTree dswTree(0.7, STOUT_WARREN_REBUILD);

    // Start all four trees with the same five keys, in the same order.
    long long keys[] = {30, 20, 40, 10, 50};
    for (long long key : keys) {
        successor.insert(key);
        randomized.insert(key);
        arrayTree.insert(key);
        dswTree.insert(key);
    }
    arrayTree.resetRebuildStats();
    dswTree.resetRebuildStats();
    long long removed[] = {30, 10, 50};
    long long inserted[] = {35, 15, 45};

    for (int i = 0; i < 3; i++) {
        std::cout << "\nPair " << i + 1 << ": delete " << removed[i]
                  << ", insert " << inserted[i] << '\n';
        successor.erase(removed[i]);
        successor.insert(inserted[i]);
        randomized.erase(removed[i]);
        randomized.insert(inserted[i]);
        arrayTree.erase(removed[i]);
        arrayTree.insert(inserted[i]);
        dswTree.erase(removed[i]);
        dswTree.insert(inserted[i]);

        std::vector<long long> expected = successor.inorder();
        if (!successor.validate() || !randomized.validate() ||
            !arrayTree.checkInvariants() || !dswTree.checkInvariants() ||
            expected.size() != 5 || randomized.inorder() != expected ||
            arrayTree.toVector() != expected || dswTree.toVector() != expected) {
            std::cerr << "Trees disagree!\n";
            return 1;
        }
        std::cout << "All four trees contain: ";
        printKeys(expected);
        std::cout << "Mean depths (root = 0):\n";
        std::cout << "  Hibbard successor: " << successor.metrics().average_depth() << '\n';
        std::cout << "  Hibbard randomized: " << randomized.metrics().average_depth() << '\n';
        std::cout << "  Scapegoat array: " << arrayTree.averageDepth() << '\n';
        std::cout << "  Scapegoat DSW: " << dswTree.averageDepth() << '\n';
    }
    return 0;
}
