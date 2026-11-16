#pragma once
#include "core.h"
#include <string>

namespace adversary {
// A shape model used only to generate a trace, never as a measured structure.
class ShapeModel {
    struct Node { Key key = 0; int left = -1; int right = -1; };
    std::vector<Node> nodes;
    int root = -1;
    int spare = -1;
public:
    explicit ShapeModel(const std::vector<Key>& keys) {
        nodes.reserve(keys.size() + 1);
        for (Key key : keys) insert(key);
    }
    void insert(Key key) {
        int* link = &root;
        while (*link != -1) {
            auto& node = nodes[*link];
            if (node.key == key) throw std::logic_error("shape duplicate");
            link = key < node.key ? &node.left : &node.right;
        }
        int index = spare;
        if (index == -1) {
            index = static_cast<int>(nodes.size());
            nodes.push_back({key, -1, -1});
        } else {
            nodes[index] = {key, -1, -1};
            spare = -1;
        }
        *link = index;
    }
    void erase(Key key) {
        int* link = &root;
        while (*link != -1 && nodes[*link].key != key) {
            auto& node = nodes[*link];
            link = key < node.key ? &node.left : &node.right;
        }
        if (*link == -1) throw std::logic_error("shape missing deletion");
        auto& node = nodes[*link];
        if (node.left != -1 && node.right != -1) {
            int* replacement = &node.right;
            while (nodes[*replacement].left != -1) replacement = &nodes[*replacement].left;
            node.key = nodes[*replacement].key;
            link = replacement;
        }
        int old = *link;
        *link = nodes[old].left != -1 ? nodes[old].left : nodes[old].right;
        spare = old;
    }
    Key target(Key fallback) const {
        if (root == -1) return fallback;
        int queue[31], depth[31];
        int begin = 0, end = 1;
        queue[0] = root;
        depth[0] = 0;
        // Scan at most depths 0..3, in left-to-right breadth-first order.
        while (begin < end && depth[begin] <= 3) {
            int currentDepth = depth[begin];
            int index = queue[begin++];
            auto& node = nodes[index];
            if (node.left != -1 && node.right != -1) return node.key;
            if (node.left != -1) { queue[end] = node.left; depth[end++] = currentDepth + 1; }
            if (node.right != -1) { queue[end] = node.right; depth[end++] = currentDepth + 1; }
        }
        return fallback;
    }
};

class Degradation {
    Random rng;
    std::vector<Key> live;
    std::vector<int> position;
    ShapeModel model;
    bool exact;
public:
    Degradation(int n, std::uint64_t seed, const std::string& mode)
        : rng(seed ^ UINT64_C(0x574f524b4c4f4144)), live(initialKeys(n, seed)),
          position(universeSize(n), -1), model(live), exact(mode == "shallow_exact") {
        if (mode != "shallow_exact" && mode != "shallow_fresh")
            throw std::invalid_argument("attack mode must be shallow_exact or shallow_fresh");
        for (int i = 0; i < n; ++i) position[live[i]] = i;
    }
    Pair next() {
        Key fallback = live[rng.below(live.size())];
        Key removed = model.target(fallback);
        int slot = position[removed];
        position[removed] = -1;
        Key inserted = removed;
        if (!exact) {
            do { inserted = static_cast<Key>(rng.below(position.size())); }
            while (position[inserted] != -1);
        }
        model.erase(removed);
        model.insert(inserted);
        live[slot] = inserted;
        position[inserted] = slot;
        return {removed, inserted};
    }
};
}
