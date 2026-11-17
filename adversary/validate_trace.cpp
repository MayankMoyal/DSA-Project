#include "cli.h"
#include "validate.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char** argv) {
    try {
        if (argc != 4) throw std::invalid_argument("usage: validate_trace n pairs trace.txt");
        int n = adversary::populationArgument(argv[1]);
        auto pairs = adversary::unsignedArgument(argv[2], "pairs");
        std::ifstream input(argv[3]);
        if (!input) throw std::runtime_error("cannot open trace");
        auto operation = [&]() {
            std::string line;
            while (std::getline(input, line)) {
                if (line.empty() || line[0] == '#') continue;
                std::istringstream row(line);
                char kind;
                adversary::Key key;
                std::string extra;
                if (!(row >> kind >> key) || (row >> extra) || (kind != 'I' && kind != 'D'))
                    throw std::runtime_error("malformed operation");
                return std::pair<char, adversary::Key>{kind, key};
            }
            throw std::runtime_error("trace ended early");
        };
        std::vector<adversary::Key> initial;
        adversary::TraceHash hash;
        for (int i = 0; i < n; ++i) {
            auto [kind, key] = operation();
            if (kind != 'I') throw std::runtime_error("setup must contain only insertions");
            initial.push_back(key);
            hash.add(kind, key);
        }
        adversary::Validator validator(initial);
        for (std::uint64_t i = 0; i < pairs; ++i) {
            auto [a, removed] = operation();
            auto [b, inserted] = operation();
            if (a != 'D' || b != 'I') throw std::runtime_error("expected delete/insert pair");
            validator.pair({removed, inserted});
            hash.add(a, removed);
            hash.add(b, inserted);
        }
        std::string line;
        while (std::getline(input, line))
            if (!line.empty() && line[0] != '#') throw std::runtime_error("extra operation after requested pairs");
        std::cout << "Validated " << pairs << " pairs; n=" << n << "; trace_hash=" << hash.get() << '\n';
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
