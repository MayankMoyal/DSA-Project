#include "hibbard_bst.h"

namespace churn {

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
}  