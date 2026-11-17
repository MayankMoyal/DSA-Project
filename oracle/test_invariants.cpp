#include "checks.h"
#include <iostream>

int main() {
    try {
        oracle::Checks checks;
        checks.audit();
        auto step = [&](char kind, oracle::Key key) {
            checks.apply({kind, key});
            checks.audit();
        };
        for (auto k : {8, 4, 12, 2, 6, 10, 14, 5, 7}) step('I', k);
        for (auto k : {4, 8, 14, 12, 2, 5, 6, 7, 10}) step('D', k);
        for (auto k : {std::numeric_limits<oracle::Key>::min(), std::numeric_limits<oracle::Key>::max()}) {
            step('D', k); step('I', k); step('I', k); step('C', k); step('D', k);
        }
        for (int k = 0; k < 128; ++k) step('I', k);
        for (int k = 127; k >= 0; --k) step('D', k);
        std::cout << "PASS four variants, audited after every fixture operation\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
