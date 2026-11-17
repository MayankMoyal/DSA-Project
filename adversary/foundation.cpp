#include "core.h"
#include "cli.h"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
    try {
        if (argc != 3) {
            throw std::invalid_argument("usage: foundation n seed");
        }
        int n = adversary::populationArgument(argv[1]);
        auto seed = adversary::unsignedArgument(argv[2], "seed");
        std::cout << "# churn-trace-v1 kind=initial n=" << n << " seed=" << seed << '\n';
        for (auto key : adversary::initialKeys(n, seed)) {
            std::cout << "I " << key << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
