#include "../src/structA/hibbard_bst.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

int main() {
    double harmonic = 0;
    for (int n = 1; n <= 8; ++n) {
        harmonic += 1.0 / n;
        std::vector<int> keys(n);
        std::iota(keys.begin(), keys.end(), 1);
        long long total = 0, permutations = 0;
        do {
            churn::HibbardBst tree;
            for (int key : keys) tree.insert(key);
            total += tree.metrics().internal_path_length;
            ++permutations;
        } while (std::next_permutation(keys.begin(), keys.end()));
        const double observed = static_cast<double>(total) / permutations / n;
        const double predicted = 2.0 * (n + 1) * harmonic / n - 4;
        if (std::abs(observed - predicted) > 1e-10) {
            std::cerr << "D4 mismatch at n=" << n << '\n';
            return 1;
        }
        std::cout << "n=" << n << " permutations=" << permutations
                  << " observed=" << observed << " exact=" << predicted << '\n';
    }
}
