#include "insertions.h"
#include <iostream>

int main(int argc, char** argv) {
    try {
        if (argc != 4) throw std::invalid_argument("usage: generate_insertions n seed pattern");
        int n = std::stoi(argv[1]);
        auto seed = std::stoull(argv[2]);
        auto keys = adversary::insertionKeys(n, seed, argv[3]);
        std::cout << "# churn-trace-v1 kind=insertions n=" << n << " seed=" << seed
                  << " mode=" << argv[3] << '\n';
        for (auto key : keys) std::cout << "I " << key << '\n';
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
