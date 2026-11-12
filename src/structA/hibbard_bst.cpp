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

}  