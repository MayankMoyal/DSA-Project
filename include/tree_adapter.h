//This file tests makes it possible for the 4 strats to be evaluated against a common sequence of operation

#pragma once

#include "src/structA/hibbard_bst.h"
#include "src/structB/scapegoat_tree.h"

namespace churn {

using Key = HibbardBst::Key;

struct Snapshot {
    std::size_t count;
    long long height;
    double average_depth;
    long long rebuild_events;
    long long nodes_rebuilt;
    long long rotations;
};

inline Snapshot inspect(const HibbardBst& tree) {
    const auto m = tree.metrics();
    return {m.size, static_cast<long long>(m.height), m.average_depth(), 0, 0, 0};
}

inline Snapshot inspect(const ScapegoatTree& tree) {
    const auto depth = tree.depthStats();
    const auto& rebuild = tree.getRebuildStats();
    return {static_cast<std::size_t>(depth.count), depth.height, depth.average(),
            rebuild.scapegoat_rebuilds + rebuild.whole_tree_rebuilds,
            rebuild.nodes_rebuilt, rebuild.stout_warren_rotations};
}

inline bool correct_shape(const HibbardBst& tree) { return tree.validate(); }
inline bool correct_shape(const ScapegoatTree& tree) { return tree.checkInvariants(); }

// Exclude initialization rebuilds from churn-only counters.
inline void begin_churn(HibbardBst&) {}
inline void begin_churn(ScapegoatTree& tree) { tree.resetRebuildStats(); }

inline std::vector<Key> keys_in_order(const HibbardBst& tree) {
    return tree.inorder();
}
inline std::vector<Key> keys_in_order(const ScapegoatTree& tree) {
    return tree.toVector();
}

inline bool add_key(HibbardBst& tree, Key key) {
    return tree.insert(key);
}
inline bool add_key(ScapegoatTree& tree, Key key) {
    // The void-returning API preserves size on duplicates.
    const auto before = tree.size();
    tree.insert(key);
    return tree.size() > before;
}

} // namespace churn
