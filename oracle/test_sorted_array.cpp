#include "sorted_array.h"
#include <iostream>

int main() {
    oracle::SortedArray a;
    constexpr auto low = std::numeric_limits<oracle::Key>::min();
    constexpr auto high = std::numeric_limits<oracle::Key>::max();
    if (a.size() || a.erase(1) || a.contains(1) ||
        !a.insert(high) || !a.insert(low) || !a.insert(0) || a.insert(low) ||
        a.keys() != std::vector<oracle::Key>{low, 0, high} ||
        !a.contains(0) || !a.erase(0) || a.erase(0) || a.contains(0) ||
        !a.erase(low) || !a.erase(high) || a.size()) {
        std::cerr << "FAIL sorted-array contract\n";
        return 1;
    }
    std::cout << "PASS sorted-array contract\n";
}
