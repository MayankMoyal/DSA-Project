#include "degradation.h"
#include <iostream>

int main(int argc, char** argv) {
    try {
        if (argc != 5) throw std::invalid_argument("usage: generate_degradation n pairs seed shallow_exact|shallow_fresh");
        int n = std::stoi(argv[1]);
        auto pairs = std::stoull(argv[2]), seed = std::stoull(argv[3]);
        adversary::Degradation gen(n, seed, argv[4]);
        std::cout << "# churn-trace-v1 kind=churn n=" << n << " pairs=" << pairs
                  << " seed=" << seed << " mode=" << argv[4] << '\n';
        for (auto key : adversary::initialKeys(n, seed)) std::cout << "I " << key << '\n';
        for (std::uint64_t i = 0; i < pairs; ++i) {
            auto p = gen.next();
            std::cout << "D " << p.removed << "\nI " << p.inserted << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
