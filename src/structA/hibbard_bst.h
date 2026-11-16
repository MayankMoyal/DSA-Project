#pragma once
#include <vector>
#include <cstddef>

namespace churn {

class HibbardBst {
public:
    using Key = long long;
    std::vector<Key> inorder() const;
    bool validate() const;
    HibbardBst() = default;
    ~HibbardBst() noexcept;

    HibbardBst(const HibbardBst&) = delete;
    HibbardBst& operator=(const HibbardBst&) = delete;

    bool insert(Key key);
    bool erase(Key key) noexcept;
    bool contains(Key key) const noexcept;
    std::size_t size() const noexcept;
    void clear() noexcept;

private:
    struct Node {
        Key key;
        Node* left = nullptr;
        Node* right = nullptr;
    };

    Node** find_link(Key key) noexcept;

    Node* root_ = nullptr;
    std::size_t size_ = 0;
};
bool HibbardBst::erase(Key key) noexcept {
    Node** link = find_link(key);
    Node* node = *link;

    if (!node) {
        return false;
    }

    if (!node->left || !node->right) {
        *link = node->left ? node->left : node->right;
    } else {
        Node** replacement_link = &node->right;
        while ((*replacement_link)->left) {
            replacement_link = &(*replacement_link)->left;
        }

        Node* replacement = *replacement_link;
        node->key = replacement->key;
        *replacement_link = replacement->right;
        node = replacement;
    }

    delete node;
    --size_;
    return true;
}

} 