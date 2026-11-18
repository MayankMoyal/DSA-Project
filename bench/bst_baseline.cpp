// Insertion and successful-search baseline (not the full churn experiment).
#include "hibbard_bst.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;

// Convert the time between two clock readings to milliseconds.
double milliseconds(Clock::time_point start, Clock::time_point end) {
    std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count();
}

void runTrial(int n, bool shuffled, int trial) {
    // Keep the original seed formula so we can reproduce the input.
    std::uint32_t seed = 20261114u + static_cast<std::uint32_t>(n * 31 + trial);
    std::vector<long long> keys;
    for (int key = 0; key < n; key++) {
        keys.push_back(key);
    }
    std::string mode = "sorted";
    if (shuffled) {
        mode = "shuffled";
        std::mt19937 insertion_rng(seed);
        std::shuffle(keys.begin(), keys.end(), insertion_rng);
    }

    // Search every key, in a separate reproducible random order.
    std::vector<long long> queries = keys;
    std::mt19937 lookup_rng(seed ^ 0x9e3779b9u);
    std::shuffle(queries.begin(), queries.end(), lookup_rng);
    churn::HibbardBst tree;

    Clock::time_point insert_start = Clock::now();
    for (long long key : keys) {
        bool inserted = tree.insert(key);
        if (!inserted) {
            throw std::runtime_error("unexpected duplicate key");
        }
    }
    Clock::time_point insert_end = Clock::now();

    int found = 0;
    Clock::time_point lookup_start = Clock::now();
    for (long long key : queries) {
        if (tree.contains(key)) {
            found++;
        }
    }
    Clock::time_point lookup_end = Clock::now();

    if (found != n || tree.size() != static_cast<std::size_t>(n)) {
        throw std::runtime_error("BST sanity check failed");
    }
    // Keep the original CSV columns in the original order.
    std::cout << mode << ',' << n << ',' << trial << ',' << seed << ','
              << milliseconds(insert_start, insert_end) << ','
              << milliseconds(lookup_start, lookup_end) << ',' << found << '\n';
}

int main() {
    int sizes[] = {1000, 2000, 4000};
    std::cout << "mode,n,trial,seed,insert_ms,lookup_ms,lookups\n";
    for (int n : sizes) {
        for (int mode = 0; mode < 2; mode++) {
            for (int trial = 1; trial <= 5; trial++) {
                bool shuffled = (mode == 1);
                runTrial(n, shuffled, trial);
            }
        }
    }
    return 0;
}
