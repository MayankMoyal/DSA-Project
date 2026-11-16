#pragma once
#include "core.h"
#include <set>

namespace adversary {
class Validator {
    std::set<Key> live;
    std::size_t population;
public:
    explicit Validator(const std::vector<Key>& initial) : live(initial.begin(), initial.end()), population(initial.size()) {
        if (!population || live.size() != population) throw std::invalid_argument("invalid initial population");
    }
    void pair(Pair p) {
        if (live.erase(p.removed) != 1) throw std::logic_error("delete must name a present key");
        if (!live.insert(p.inserted).second) throw std::logic_error("insert must name an absent key");
        if (live.size() != population) throw std::logic_error("population changed");
    }
    std::vector<Key> keys() const { return {live.begin(), live.end()}; }
};

// Accepts bool- or void-returning insert APIs. Validity is checked before replay.
template<class Tree> void replayPair(Tree& tree, Pair p, std::size_t n) {
    if (!tree.erase(p.removed)) throw std::logic_error("replayed deletion failed");
    tree.insert(p.inserted);
    if (static_cast<std::size_t>(tree.size()) != n) throw std::logic_error("replayed population mismatch");
}
}
