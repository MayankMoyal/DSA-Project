#include "baseline.h"
#include "degradation.h"
#include "insertions.h"
#include "validate.h"
#include <algorithm>
#include <iostream>
#include <limits>

void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
template<class Generator> void check(const std::string& mode) {
    for (int n : {1, 2, 7, 31, 128}) {
        auto initial = adversary::initialKeys(n, 1907);
        adversary::Validator oracle(initial);
        Generator first(n, 1907, mode), second(n, 1907, mode);
        for (int i = 0; i < 10000; ++i) {
            auto a = first.next(), b = second.next();
            require(a.removed == b.removed && a.inserted == b.inserted, "trace is not reproducible");
            if (mode == "exact" || mode == "shallow_exact")
                require(a.removed == a.inserted, "exact-fit changed key set");
            oracle.pair(a);
        }
        if (mode == "exact" || mode == "shallow_exact") {
            std::sort(initial.begin(), initial.end());
            require(oracle.keys() == initial, "exact-fit set changed");
        }
    }
}
int main() {
    try {
        adversary::Random zero(0);
        require(zero.next() == UINT64_C(0xe220a8397b1dcdaf), "SplitMix64 golden value mismatch");
        check<adversary::Baseline>("random");
        check<adversary::Baseline>("exact");
        check<adversary::Degradation>("shallow_exact");
        check<adversary::Degradation>("shallow_fresh");
        for (auto mode : {"ascending", "descending", "shuffle", "extremes", "blocks"})
            for (int n : {1, 2, 31, 32, 33, 4000}) {
                auto keys = adversary::insertionKeys(n, 77, mode);
                require(keys == adversary::insertionKeys(n, 77, mode), "insertion order changed");
                std::sort(keys.begin(), keys.end());
                for (int i = 0; i < n; ++i) require(keys[i] == i, "insertion pattern is not a permutation");
            }
        for (int n : {0, -1, 1000001}) {
            bool rejected = false;
            try { adversary::initialKeys(n, 1); } catch (const std::invalid_argument&) { rejected = true; }
            require(rejected, "invalid population accepted");
        }
        bool rejected = false;
        try { adversary::Validator v({1, 2}); v.pair({1, 2}); }
        catch (const std::logic_error&) { rejected = true; }
        require(rejected, "duplicate insertion accepted by validator");
        rejected = false;
        try { adversary::Validator v({1, 2}); v.pair({9, 3}); }
        catch (const std::logic_error&) { rejected = true; }
        require(rejected, "absent deletion accepted by validator");
        adversary::Validator boundary({std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max()});
        boundary.pair({std::numeric_limits<long long>::min(), 0});
        boundary.pair({std::numeric_limits<long long>::max(), std::numeric_limits<long long>::min()});
        adversary::ShapeModel shape({4, 2, 6, 1, 3, 5, 7});
        require(shape.target(1) == 4, "unexpected shallow target");
        shape.erase(4); shape.insert(4);
        require(shape.target(1) == 5, "successor directional example failed");
        std::cout << "Adversary checks passed: 200000 generated pairs, all five insertion patterns, invalid inputs and boundary keys\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
