#include "trace.h"
#include <iostream>

namespace {
using Trace = std::vector<oracle::Op>;
oracle::Outcome run(const Trace& trace) {
    return oracle::replay([&](auto emit) {
        for (const auto op : trace) if (!emit(op)) break;
    });
}

Trace omit(const Trace& trace, std::size_t first, std::size_t last) {
    Trace candidate;
    candidate.reserve(trace.size() - (last - first));
    candidate.insert(candidate.end(), trace.begin(), trace.begin() + first);
    candidate.insert(candidate.end(), trace.begin() + last, trace.end());
    return candidate;
}

Trace minimize(Trace trace, const oracle::Failure& target) {
    // After each success restart on the shorter sequence. At unit width,
    // testing every position without a success certifies deletion-1-minimality.
    std::size_t width = trace.size() / 2;
    while (!trace.empty()) {
        width = std::max<std::size_t>(1, std::min(width, trace.size()));
        bool reduced = false;
        for (std::size_t first = 0; first < trace.size(); first += width) {
            auto candidate = omit(trace, first, std::min(trace.size(), first + width));
            auto outcome = run(candidate);
            if (outcome.failure && outcome.failure->variant == target.variant &&
                outcome.failure->check == target.check) {
                candidate.resize(outcome.operations);
                trace = std::move(candidate);
                reduced = true;
                break;
            }
        }
        if (reduced) continue;
        if (width == 1) break;
        width /= 2;
    }
    return trace;
}
}

int main(int argc, char** argv) {
    try {
        if (argc != 3) throw std::invalid_argument("usage: shrink INPUT NEW_OUTPUT");
        if (std::filesystem::exists(argv[2])) throw std::invalid_argument("output already exists");
        Trace trace;
        oracle::read_trace(argv[1], [&](oracle::Op op) { trace.push_back(op); return true; });
        auto original = run(trace);
        if (!original.failure) throw std::invalid_argument("input passes; no failing case to shrink");
        trace.resize(original.operations);
        auto minimal = minimize(std::move(trace), *original.failure);
        auto verified = run(minimal);
        if (!verified.failure || verified.failure->variant != original.failure->variant ||
            verified.failure->check != original.failure->check)
            throw std::runtime_error("replay of minimal trace disagrees");
        std::ofstream out(argv[2]);
        out.exceptions(std::ios::badbit | std::ios::failbit);
        out << "# " << original.failure->what() << " policy_seed=" << oracle::policy_seed << '\n';
        for (const auto op : minimal) oracle::write_op(out, op);
        out.close();
        std::cout << "PASS shrink " << original.failure->what() << " " << original.operations
                  << " -> " << minimal.size() << " operations (1-minimal)\n";
    } catch (const std::exception& e) {
        std::cerr << "ERROR " << e.what() << '\n';
        return 2;
    }
}
