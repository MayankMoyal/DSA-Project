#include "scapegoat_tree.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace std;

ScapegoatTree::ScapegoatTree(double a) {
    if (a <= 0.5 || a >= 1.0) {
        throw invalid_argument("alpha must be between 0.5 and 1");
    }

    root = nullptr;
    alpha = a;
    n = 0;
    max_n = 0;
}

ScapegoatTree::~ScapegoatTree() {
    destroy(root);
}

void ScapegoatTree::destroy(Node* v) {
    if (v == nullptr) return;
    destroy(v->left);
    destroy(v->right);
    delete v;
}

Node* ScapegoatTree::insertNode(Node* v, ll key) {
    if (v == nullptr) {
        return new Node(key);
    }

    if (key < v->key) {
        v->left = insertNode(v->left, key);
    }
    else if (key > v->key) {
        v->right = insertNode(v->right, key);
    }

    return v;
}

void ScapegoatTree::insert(ll key) {
    path.clear();

    Node* cur = root;
    Node* parent = nullptr;

    while (cur != nullptr) {
        parent = cur;
        path.push_back(cur);

        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return; 
    }

    Node* fresh = new Node(key);

    if (parent == nullptr) root = fresh;
    else if (key < parent->key) parent->left = fresh;
    else parent->right = fresh;

    path.push_back(fresh);
    n++;
    if (n > max_n) max_n = n;

    int depth = static_cast<int>(path.size()) - 1;

    if (depth > static_cast<int>(logInvAlpha(max_n))) {
        rebuildAtScapegoat();
    }

}

bool ScapegoatTree::contains(ll key) const {
    Node* cur = root;

    while (cur != nullptr) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return true;
    }

    return false;
}

void ScapegoatTree::printNode(Node* v) const {
    if (v == nullptr) return;

    printNode(v->left);
    cout << v->key << " ";
    printNode(v->right);
}

void ScapegoatTree::printInorder() const {
    printNode(root);
    cout << '\n';
}

int ScapegoatTree::heightOf(Node* v) const {
    if (v == nullptr) return -1;

    int leftHeight = heightOf(v->left);
    int rightHeight = heightOf(v->right);

    return 1 + max(leftHeight, rightHeight);
}

int ScapegoatTree::height() const {
    return heightOf(root);
}

int ScapegoatTree::size() const {
    return n;
}

int ScapegoatTree::countNodes(Node* v) const {
    if (v == nullptr) return 0;
    return 1 + countNodes(v->left) + countNodes(v->right);
}

double ScapegoatTree::logInvAlpha(int value) const {
    if (value <= 1) return 0.0;
    return log(static_cast<double>(value)) / log(1.0 / alpha);
}


void ScapegoatTree::rebuildAtScapegoat() {
    if (path.size() < 2) return;

    for (int i = static_cast<int>(path.size())-2; i>=0; i--) {
        Node* parent = path[i];
        Node* child = path[i + 1];

        int parentSize = countNodes(parent);
        int childSize = countNodes(child);

        if (childSize > alpha * parentSize) {
            Node* rebuilt = rebuildSubtree(parent);

            if (i == 0) {
                root = rebuilt;
            } 
            else {
                Node* grandparent = path[i-1];

                if (grandparent->left == parent) {
                    grandparent->left = rebuilt;
                } 
                else {
                    grandparent->right = rebuilt;
                }
            }
            return;
        }
    }
}

void ScapegoatTree::flatten(Node* v, vector<Node*>& nodes) {
    if (v == nullptr) return;

    flatten(v->left, nodes);
    nodes.push_back(v);
    flatten(v->right, nodes);
}

Node* ScapegoatTree::buildBalanced(vector<Node*>& nodes, int low, int high) {
    if (low > high) return nullptr;

    int mid = low + (high - low) / 2;
    Node* v = nodes[mid];

    v->left = buildBalanced(nodes, low, mid - 1);
    v->right = buildBalanced(nodes, mid + 1, high);

    return v;
}

Node* ScapegoatTree::rebuildSubtree(Node* v) {
    vector<Node*> nodes;
    flatten(v, nodes);
    return buildBalanced(nodes, 0, static_cast<int>(nodes.size()) - 1);
}
