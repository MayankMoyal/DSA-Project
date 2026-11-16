#pragma once
#include <vector>
#include <cstddef>
#include <cstdint>
#include <random>

namespace churn {
enum class DeletionPolicy {
    AlwaysSuccessor,
    RandomPredecessorOrSuccessor
};
struct TreeMetrics {
    std::size_t size = 0;
    std::uint64_t internal_path_length = 0;
    std::ptrdiff_t height = -1;

    double average_depth() const noexcept;
};
class HibbardBst {
public:
    using Key = long long;
    std::vector<Key> inorder() const;
    bool validate() const;
    explicit HibbardBst(
        DeletionPolicy policy = DeletionPolicy::AlwaysSuccessor,
        std::uint64_t policy_seed = 0
    );
    ~HibbardBst() noexcept;
    TreeMetrics metrics() const;

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
    bool use_successor() noexcept;

    Node* root_ = nullptr;
    std::size_t size_ = 0;
    
    DeletionPolicy policy_;
    std::mt19937_64 policy_rng_;
};
}