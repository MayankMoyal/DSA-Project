#include "trace.h"
#include "../adversary/baseline.h"
#include <iostream>

namespace {
using namespace oracle;

template<class Source>
int check(Source source) {
    const auto result = replay(source);
    if (!result.failure) {
        std::cout << "PASS operations=" << result.operations << " variants=4 policy_seed="
                  << policy_seed << " audit_interval=" << audit_interval << '\n';
        return 0;
    }
    std::cerr << "FAIL " << result.failure->what() << " at operation " << result.operations << '\n';
    std::filesystem::path dir;
    for (std::size_t i = 1; ; ++i) {
        dir = "oracle-failure-" + std::to_string(i);
        if (std::filesystem::create_directory(dir)) break;
    }
    const auto path = dir / "prefix.txt";
    std::ofstream out(path);
    out.exceptions(std::ios::badbit | std::ios::failbit);
    out << "# " << result.failure->what() << " policy_seed=" << policy_seed << '\n';
    std::size_t written = 0;
    if (result.operations) {
        source([&](Op op) {
            write_op(out, op);
            return ++written < result.operations;
        });
    }
    out.close();
    std::cerr << "Saved " << path.string() << '\n';
    return 1;
}
}

int main(int argc, char** argv) {
    try {
        std::string command = argc > 1 ? argv[1] : "";
        if (command == "replay" && argc == 3) {
            return check([&](auto emit) { oracle::read_trace(argv[2], emit); });
        }
        if (command == "mixed" && argc == 4) {
            const auto seed = oracle::number<std::uint64_t>(argv[2]);
            const auto count = oracle::number<std::size_t>(argv[3]);
            if (!count) throw std::invalid_argument("operation count must be positive");
            std::cout << "mixed seed=" << seed << " count=" << count << '\n';
            return check([&](auto emit) {
                adversary::Random rng(seed);
                constexpr char ops[] = {'I', 'D', 'C'};
                for (std::size_t i = 0; i < count; ++i) {
                    const char op = ops[rng.below(3)];
                    const auto key = static_cast<oracle::Key>(rng.below(2049)) - 1024;
                    if (!emit({op, key})) return;
                }
            });
        }
        if (command == "churn" && argc == 6) {
            const int n = oracle::number<int>(argv[2]);
            const auto pairs = oracle::number<std::size_t>(argv[3]);
            const auto seed = oracle::number<std::uint64_t>(argv[4]);
            const std::string mode = argv[5];
            std::cout << "churn n=" << n << " pairs=" << pairs << " seed=" << seed << " mode=" << mode << '\n';
            return check([&](auto emit) {
                adversary::Baseline workload(n, seed, mode);
                for (const auto key : adversary::initialKeys(n, seed))
                    if (!emit({ 'I', key })) return;
                for (std::size_t i = 0; i < pairs; ++i) {
                    const auto p = workload.next();
                    if (!emit({'D', p.removed}) || !emit({'I', p.inserted})) return;
                }
            });
        }
        throw std::invalid_argument("usage: oracle mixed SEED COUNT | churn N PAIRS SEED random|exact | replay FILE");
    } catch (const std::exception& e) {
        std::cerr << "ERROR " << e.what() << '\n';
        return 2;
    }
}
