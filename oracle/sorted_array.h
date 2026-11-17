#pragma once
#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

namespace oracle {
using Key = long long;
static_assert(std::numeric_limits<Key>::digits == 63, "signed 64-bit keys required");

class SortedArray {
    std::vector<Key> keys_;
public:
    bool insert(Key key) {
        auto position = std::lower_bound(keys_.begin(), keys_.end(), key);
        if (position != keys_.end() && *position == key) return false;
        keys_.insert(position, key);
        return true;
    }

    bool erase(Key key) {
        auto position = std::lower_bound(keys_.begin(), keys_.end(), key);
        if (position == keys_.end() || *position != key) return false;
        keys_.erase(position);
        return true;
    }

    bool contains(Key key) const {
        return std::binary_search(keys_.begin(), keys_.end(), key);
    }

    std::size_t size() const { return keys_.size(); }
    const std::vector<Key>& keys() const { return keys_; }
};
}
