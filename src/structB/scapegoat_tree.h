#ifndef SCAPEGOAT_TREE_H
#define SCAPEGOAT_TREE_H

#include <vector>
using namespace std;

typedef long long ll;

struct Node {
    ll key;
    Node* left;
    Node* right;

    Node(ll k) {
        key = k;
        left = nullptr;
        right = nullptr;
    }
};

enum RebuildMode {
    ARRAY_REBUILD,
    STOUT_WARREN_REBUILD
};

struct RebuildStats {
    long long insertions = 0;
    long long deletions = 0;
    long long scapegoat_rebuilds = 0;
    long long whole_tree_rebuilds = 0;
    long long nodes_rebuilt = 0;
    long long array_rebuilds = 0;
    long long stout_warren_rebuilds = 0;
    long long stout_warren_rotations = 0;
};

struct DepthStats {
    long long count = 0;
    long long sum_depth = 0;
    int height = -1;

    double average() const {
        if (count == 0) return 0.0;
        return static_cast<double>(sum_depth) / count;
    }
};

class ScapegoatTree {
public:
    ScapegoatTree(double a = 0.7,
                  RebuildMode mode = ARRAY_REBUILD);
    ~ScapegoatTree();

    ScapegoatTree(const ScapegoatTree&) = delete;
    ScapegoatTree& operator=(const ScapegoatTree&) = delete;

    void insert(ll key);
    bool erase(ll key);
    bool contains(ll key) const;
    void printInorder() const;

    int height() const;
    int size() const;
    double averageDepth() const;
    DepthStats depthStats() const;

    void setRebuildMode(RebuildMode mode);
    RebuildMode getRebuildMode() const;

    void rebuildAll();
    int getMaxN() const;
    vector<ll> toVector() const;
    bool checkInvariants() const;
    const RebuildStats& getRebuildStats() const;
    void resetRebuildStats();

private:
    Node* root;
    double alpha;
    int n;
    int max_n;
    RebuildMode rebuildMode;

    vector<Node*> path;
    RebuildStats stats;

    Node* findMin(Node* v) const;
    Node* eraseNode(Node* v, ll key, bool& erased);

    void printNode(Node* v) const;
    int heightOf(Node* v) const;
    int countNodes(Node* v) const;
    void destroy(Node* v);

    void collectDepthStats(Node* v, int depth, DepthStats& result) const;
    void collectInorder(Node* v, vector<ll>& result) const;

    double logInvAlpha(int value) const;
    void rebuildAtScapegoat();
    Node* rebuildSubtree(Node* v);

    void flatten(Node* v, vector<Node*>& nodes);
    Node* buildBalanced(vector<Node*>& nodes, int low, int high);

    void treeToVine(Node* pseudoRoot, int& nodeCount);
    void compress(Node* pseudoRoot, int rotations);
    Node* rebuildStoutWarren(Node* v, int nodeCount);
};

#endif
