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

class ScapegoatTree {
public:
    ScapegoatTree(double a = 0.7);
    ~ScapegoatTree();

    void insert(ll key);
    bool contains(ll key) const;
    void printInorder() const;
    int height() const;
    int size() const;

private:
    Node* root;
    double alpha;
    int n;
    int max_n;

    vector<Node*> path;

    Node* insertNode(Node* v, ll key);
    void printNode(Node* v) const;
    int heightOf(Node* v) const;
    int countNodes(Node* v) const;
    void destroy(Node* v);

    double logInvAlpha(int value) const;
    void rebuildAtScapegoat();
    Node* rebuildSubtree(Node* v);
    void flatten(Node* v, vector<Node*>& nodes);
    Node* buildBalanced(vector<Node*>& nodes, int lo, int hi);
};

#endif
