#pragma once
#include "core.h"
#include <algorithm>
#include <string>

namespace adversary {
inline std::vector<Key> insertionKeys(int n, std::uint64_t seed, const std::string& mode) {
    universeSize(n);
    std::vector<Key> keys(n);
    for (int i = 0; i < n; ++i) keys[i] = i;
    if (mode == "ascending") return keys;
    if (mode == "descending") {
        std::reverse(keys.begin(), keys.end());
    } else if (mode == "shuffle") {
        Random rng(seed);
        rng.shuffle(keys);
    } else if (mode == "extremes") {
        int low = 0, high = n - 1;
        for (int i = 0; i < n; ++i) keys[i] = i % 2 ? high-- : low++;
    } else if (mode == "blocks") {
        for (int start = 0; start < n; start += 32)
            if ((start / 32) % 2) std::reverse(keys.begin() + start, keys.begin() + std::min(n, start + 32));
    } else {
        throw std::invalid_argument("insertion mode must be ascending, descending, shuffle, extremes or blocks");
    }
    return keys;
}
}
