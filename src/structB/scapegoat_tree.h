#ifndef SCAPEGOAT_TREE_H
#define SCAPEGOAT_TREE_H
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
    ScapegoatTree();
    ~ScapegoatTree();

    void insert(ll key);         
    bool contains(ll key) const;
    void printInorder() const;          
    int height() const;                

private:
    Node* root;

    Node* insertNode(Node* v, ll key);
    void printNode(Node* v) const;
    int heightOf(Node* v) const;
    void destroy(Node* v);

};

#endif




