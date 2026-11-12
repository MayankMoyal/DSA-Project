#pragma once

#include <cstddef>

namespace churn {

class HibbardBst {
public:
    using Key = long long;

    HibbardBst() = default;
    ~HibbardBst() noexcept;

    HibbardBst(const HibbardBst&) = delete;
    HibbardBst& operator=(const HibbardBst&) = delete;

    bool insert(Key key);
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

} 