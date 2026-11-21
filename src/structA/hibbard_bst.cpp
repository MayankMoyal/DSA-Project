#include "hibbard_bst.h"
#include <utility>
namespace churn {
HibbardBst::HibbardBst(DeletionPolicy policy, std::uint64_t policy_seed)
    : policy_(policy), policy_rng_(policy_seed) {}

bool HibbardBst::use_successor() noexcept {
    return policy_ == DeletionPolicy::AlwaysSuccessor ||
           (policy_rng_() & 1ULL) == 0;
}
HibbardBst::~HibbardBst() noexcept {
    clear();
}

HibbardBst::Node** HibbardBst::find_link(Key key) noexcept {
    Node** link = &root_;

    while (*link && (*link)->key != key) {
        Node* node = *link;
        link = key < node->key ? &node->left : &node->right;
    }

    return link;
}

bool HibbardBst::insert(Key key) {
    Node** link = find_link(key);
    if (*link) {
        return false;
    }

    *link = new Node{key};
    ++size_;
    return true;
}

bool HibbardBst::contains(Key key) const noexcept {
    const Node* node = root_;

    while (node && node->key != key) {
        node = key < node->key ? node->left : node->right;
    }

    return node != nullptr;
}

std::size_t HibbardBst::size() const noexcept {
    return size_;
}

void HibbardBst::clear() noexcept {
    while (root_) {
        if (root_->left) {
            Node* left = root_->left;
            root_->left = left->right;
            left->right = root_;
            root_ = left;
        } else {
            Node* old_root = root_;
            root_ = root_->right;
            delete old_root;
        }
    }

    size_ = 0;
}
std::vector<HibbardBst::Key> HibbardBst::inorder() const {
    std::vector<Key> keys;
    keys.reserve(size_);

    std::vector<const Node*> pending;
    const Node* node = root_;

    while (node || !pending.empty()) {
        while (node) {
            pending.push_back(node);
            node = node->left;
        }

        node = pending.back();
        pending.pop_back();
        keys.push_back(node->key);
        node = node->right;
    }

    return keys;
}

bool HibbardBst::validate() const {
    struct Frame {
        const Node* node;
        const Node* lower;
        const Node* upper;
    };

    std::vector<Frame> pending;
    if (root_) {
        pending.push_back({root_, nullptr, nullptr});
    }

    std::size_t visited = 0;

    while (!pending.empty()) {
        const auto [node, lower, upper] = pending.back();
        pending.pop_back();

        if (++visited > size_ ||
            (lower && node->key <= lower->key) ||
            (upper && node->key >= upper->key)) {
            return false;
        }

        if (node->right) {
            pending.push_back({node->right, node, upper});
        }
        if (node->left) {
            pending.push_back({node->left, lower, node});
        }
    }

    return visited == size_;
}
double TreeMetrics::average_depth() const noexcept {
    if (size == 0) {
        return 0.0;
    }
    return static_cast<double>(internal_path_length) / static_cast<double>(size);
}

TreeMetrics HibbardBst::metrics() const {
    TreeMetrics result;
    std::vector<std::pair<const Node*, std::ptrdiff_t>> pending;

    if (root_) {
        pending.emplace_back(root_, 0);
    }

    while (!pending.empty()) {
        const auto [node, depth] = pending.back();
        pending.pop_back();

        ++result.size;
        result.internal_path_length += static_cast<std::uint64_t>(depth);
        if (depth > result.height) {
            result.height = depth;
        }

        if (node->right) {
            pending.emplace_back(node->right, depth + 1);
        }
        if (node->left) {
            pending.emplace_back(node->left, depth + 1);
        }
    }

    return result;
}
bool HibbardBst::erase(Key key) noexcept {
    Node** link = find_link(key);
    Node* node = *link;

    if (!node) {
        return false;
    }

    if (!node->left || !node->right) {
        *link = node->left ? node->left : node->right;
    } else {
    const bool successor = use_successor();
    Node** replacement_link = successor ? &node->right : &node->left;

    while (successor ? (*replacement_link)->left : (*replacement_link)->right) {
        replacement_link = successor
            ? &(*replacement_link)->left
            : &(*replacement_link)->right;
    }

    Node* replacement = *replacement_link;
    node->key = replacement->key;
    *replacement_link = successor ? replacement->right : replacement->left;
    node = replacement;
}
    delete node;
    --size_;
    return true;
}  
}
// Read-only breadth-first snapshot: key, left index, right index (-1 = absent).
std::vector<std::array<long long, 3>> churn::HibbardBst::snapshot() const {
    std::vector<std::array<long long, 3>> result;
    if (!root_) return result;
    std::vector<const Node*> queue{root_};
    for (std::size_t i = 0; i < queue.size(); ++i) {
        const auto* node = queue[i];
        long long left = -1, right = -1;
        if (node->left) { left = queue.size(); queue.push_back(node->left); }
        if (node->right) { right = queue.size(); queue.push_back(node->right); }
        result.push_back({node->key, left, right});
    }
    return result;
}
