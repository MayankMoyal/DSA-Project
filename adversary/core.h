#pragma once

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace adversary {
using Key = long long;
static_assert(std::numeric_limits<Key>::digits == 63, "64-bit keys required");

class Random {
    std::uint64_t state;
public:
    explicit Random(std::uint64_t seed) : state(seed) {}
    std::uint64_t next() {
        std::uint64_t z = (state += UINT64_C(0x9e3779b97f4a7c15));
        z = (z ^ (z >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
        z = (z ^ (z >> 27)) * UINT64_C(0x94d049bb133111eb);
        return z ^ (z >> 31);
    }
    std::uint64_t below(std::uint64_t bound) {
        if (!bound) throw std::invalid_argument("random bound is zero");
        const auto threshold = -bound % bound;
        std::uint64_t x;
        do { x = next(); } while (x < threshold);
        return x % bound;
    }
    template<class T> void shuffle(std::vector<T>& values) {
        for (std::size_t i = values.size(); i > 1; --i)
            std::swap(values[i - 1], values[below(i)]);
    }
};

struct Pair { Key removed; Key inserted; };

inline int universeSize(int n) {
    if (n < 1 || n > 1000000)
        throw std::invalid_argument("n must be in [1,1000000]");
    return 16 * n;
}

inline std::vector<Key> initialKeys(int n, std::uint64_t seed) {
    const int universe = universeSize(n);
    std::vector<Key> keys(universe);
    for (int i = 0; i < universe; ++i) keys[i] = i;
    Random rng(seed ^ UINT64_C(0x494e495449414c));
    rng.shuffle(keys);
    keys.resize(n);
    return keys;
}

class TraceHash {
    std::uint64_t value = UINT64_C(14695981039346656037);
public:
    void add(char operation, Key key) {
        value = (value ^ static_cast<unsigned char>(operation)) * UINT64_C(1099511628211);
        auto bits = static_cast<std::uint64_t>(key);
        for (int i = 0; i < 8; ++i) {
            value = (value ^ (bits & 255)) * UINT64_C(1099511628211);
            bits >>= 8;
        }
    }
    std::uint64_t get() const { return value; }
};
}
