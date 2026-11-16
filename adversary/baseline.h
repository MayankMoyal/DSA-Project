#pragma once
#include "core.h"
#include <string>

namespace adversary {
class Baseline {
    Random rng;
    std::vector<Key> live;
    std::vector<int> position;
    bool exact;
public:
    Baseline(int n, std::uint64_t seed, const std::string& mode)
        : rng(seed ^ UINT64_C(0x574f524b4c4f4144)), live(initialKeys(n, seed)),
          position(universeSize(n), -1), exact(mode == "exact") {
        if (mode != "random" && mode != "exact")
            throw std::invalid_argument("baseline mode must be random or exact");
        for (int i = 0; i < n; ++i) position[live[i]] = i;
    }
    Pair next() {
        auto slot = rng.below(live.size());
        Key removed = live[slot];
        position[removed] = -1;
        Key inserted = removed;
        if (!exact) {
            do { inserted = static_cast<Key>(rng.below(position.size())); }
            while (position[inserted] != -1);
        }
        live[slot] = inserted;
        position[inserted] = static_cast<int>(slot);
        return {removed, inserted};
    }
};
}
