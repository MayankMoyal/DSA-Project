#include "core.h"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    try {
        if (argc != 3) throw std::invalid_argument("usage: foundation n seed");
        int n = std::stoi(argv[1]);
        auto seed = std::stoull(argv[2]);
        std::cout << "# churn-trace-v1 kind=initial n=" << n << " seed=" << seed << '\n';
        for (auto key : adversary::initialKeys(n, seed)) std::cout << "I " << key << '\n';
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
