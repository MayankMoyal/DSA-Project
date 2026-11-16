#include "scapegoat_tree.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace std;

ScapegoatTree::ScapegoatTree(double a, RebuildMode mode) {
    if (!isfinite(a) || a <= 0.5 || a >= 1.0) {
        throw invalid_argument("alpha must be between 0.5 and 1");
    }

    root = nullptr;
    alpha = a;
    n = 0;
    max_n = 0;
    rebuildMode = mode;
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
    stats.insertions++;

    if (n > max_n) max_n = n;

    int depth = static_cast<int>(path.size()) - 1;

    if (depth > static_cast<int>(logInvAlpha(max_n))) {
        rebuildAtScapegoat();
    }
}

Node* ScapegoatTree::findMin(Node* v) const {
    while (v != nullptr && v->left != nullptr) {
        v = v->left;
    }
    return v;
}

Node* ScapegoatTree::eraseNode(Node* v, ll key, bool& erased) {
    if (v == nullptr) return nullptr;

    if (key < v->key) {
        v->left = eraseNode(v->left, key, erased);
    }
    else if (key > v->key) {
        v->right = eraseNode(v->right, key, erased);
    }
    else {
        erased = true;

        if (v->left == nullptr) {
            Node* next = v->right;
            delete v;
            return next;
        }
        if (v->right == nullptr) {
            Node* next = v->left;
            delete v;
            return next;
        }

        Node* successor = findMin(v->right);
        v->key = successor->key;

        bool removedSuccessor = false;
        v->right = eraseNode(v->right, successor->key, removedSuccessor);
    }

    return v;
}

bool ScapegoatTree::erase(ll key) {
    bool erased = false;
    root = eraseNode(root, key, erased);

    if (!erased) return false;

    n--;
    stats.deletions++;

    if (n < alpha * max_n) {
        rebuildAll();
    }

    return true;
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

    return max(leftHeight, rightHeight) + 1;
}

int ScapegoatTree::height() const {
    return heightOf(root);
}

int ScapegoatTree::size() const {
    return n;
}

int ScapegoatTree::getMaxN() const {
    return max_n;
}

void ScapegoatTree::collectInorder(Node* v, vector<ll>& result) const {
    if (v == nullptr) return;
    collectInorder(v->left, result);
    result.push_back(v->key);
    collectInorder(v->right, result);
}

vector<ll> ScapegoatTree::toVector() const {
    vector<ll> result;
    result.reserve(n);
    collectInorder(root, result);
    return result;
}

bool ScapegoatTree::checkInvariants() const {
    auto inspect = [&](auto&& self, Node* v, bool hasLow, ll low,
                       bool hasHigh, ll high) -> pair<bool, int> {
        if (v == nullptr) return {true, 0};
        if ((hasLow && v->key <= low) || (hasHigh && v->key >= high)) {
            return {false, 0};
        }
        auto left = self(self, v->left, hasLow, low, true, v->key);
        if (!left.first) return {false, 0};
        auto right = self(self, v->right, true, v->key, hasHigh, high);
        if (!right.first) return {false, 0};
        return {true, 1 + left.second + right.second};
    };

    auto result = inspect(inspect, root, false, 0, false, 0);
    if (!result.first || result.second != n || max_n < n) return false;
    if (n == 0) return root == nullptr && max_n == 0;

    int allowedHeight = static_cast<int>(logInvAlpha(max_n));
    return height() <= allowedHeight;
}

int ScapegoatTree::countNodes(Node* v) const {
    if (v == nullptr) return 0;
    return 1 + countNodes(v->left) + countNodes(v->right);
}

void ScapegoatTree::collectDepthStats(
    Node* v, int depth, DepthStats& result) const {

    if (v == nullptr) return;

    result.count++;
    result.sum_depth += depth;
    result.height = max(result.height, depth);

    collectDepthStats(v->left, depth + 1, result);
    collectDepthStats(v->right, depth + 1, result);
}

DepthStats ScapegoatTree::depthStats() const {
    DepthStats result;
    collectDepthStats(root, 0, result);
    return result;
}

double ScapegoatTree::averageDepth() const {
    return depthStats().average();
}

double ScapegoatTree::logInvAlpha(int value) const {
    if (value <= 1) return 0.0;
    return log(static_cast<double>(value)) / log(1.0 / alpha);
}

void ScapegoatTree::rebuildAtScapegoat() {
    if (path.size() < 2) return;

    int childSize = 1;

    for (int i = static_cast<int>(path.size()) - 2; i >= 0; i--) {
        Node* parent = path[i];
        Node* child = path[i + 1];
        Node* sibling = (parent->left == child) ? parent->right : parent->left;
        int parentSize = childSize + countNodes(sibling) + 1;

        if (childSize > alpha * parentSize) {
            Node* rebuilt = rebuildSubtree(parent);
            stats.scapegoat_rebuilds++;

            if (i == 0) {
                root = rebuilt;
            } 
            
            else {
                Node* grandparent = path[i - 1];
                if (grandparent->left == parent) grandparent->left = rebuilt;
                else grandparent->right = rebuilt;
            }
            return;
        }
        childSize = parentSize;
    }
}

void ScapegoatTree::flatten(Node* v, vector<Node*>& nodes) {
    if (v == nullptr) return;

    flatten(v->left, nodes);
    nodes.push_back(v);
    flatten(v->right, nodes);
}

Node* ScapegoatTree::buildBalanced(
    vector<Node*>& nodes, int low, int high) {

    if (low > high) return nullptr;

    int mid = low + (high - low) / 2;
    Node* v = nodes[mid];

    v->left = buildBalanced(nodes, low, mid - 1);
    v->right = buildBalanced(nodes, mid + 1, high);

    return v;
}


void ScapegoatTree::treeToVine(Node* pseudoRoot, int& nodeCount) {
    Node* tail = pseudoRoot;
    Node* rest = tail->right;

    while (rest != nullptr) {
        if (rest->left != nullptr) {
            Node* child = rest->left;
            rest->left = child->right;
            child->right = rest;
            tail->right = child;
            rest = child;
            stats.stout_warren_rotations++;
        } 
        
        else {
            nodeCount++;
            tail = rest;
            rest = rest->right;
        }
    }
}

void ScapegoatTree::compress(Node* pseudoRoot, int rotations) {
    Node* scanner = pseudoRoot;

    for (int i = 0; i < rotations; i++) {
        Node* child = scanner->right;
        if (child == nullptr || child->right == nullptr) break;

        Node* grandchild = child->right;
        scanner->right = grandchild;
        child->right = grandchild->left;
        grandchild->left = child;

        scanner = grandchild;
        stats.stout_warren_rotations++;
    }
}

Node* ScapegoatTree::rebuildStoutWarren(Node* v, int nodeCount) {
    Node pseudoRoot(0);
    pseudoRoot.right = v;

    int counted = 0;
    treeToVine(&pseudoRoot, counted);
    if (counted != nodeCount) {
        throw logic_error("Stout-Warren rebuild node count mismatch");
    }

    int m = 1;
    while (m <= nodeCount + 1) m *= 2;
    m = m/2 - 1;

    compress(&pseudoRoot, nodeCount - m);

    while (m > 1) {
        m /= 2;
        compress(&pseudoRoot, m);
    }

    return pseudoRoot.right;
}

Node* ScapegoatTree::rebuildSubtree(Node* v) {
    int subtreeSize = countNodes(v);
    stats.nodes_rebuilt += subtreeSize;

    if (rebuildMode == ARRAY_REBUILD) {
        vector<Node*> nodes;
        nodes.reserve(subtreeSize);
        flatten(v, nodes);
        stats.array_rebuilds++;
        return buildBalanced(nodes, 0, static_cast<int>(nodes.size()) - 1);
    }

    stats.stout_warren_rebuilds++;
    return rebuildStoutWarren(v, subtreeSize);
}

void ScapegoatTree::rebuildAll() {
    if (root != nullptr) {
        root = rebuildSubtree(root);
    }
    stats.whole_tree_rebuilds++;
    max_n = n;
}

void ScapegoatTree::setRebuildMode(RebuildMode mode) {
    rebuildMode = mode;
}

RebuildMode ScapegoatTree::getRebuildMode() const {
    return rebuildMode;
}

const RebuildStats& ScapegoatTree::getRebuildStats() const {
    return stats;
}

void ScapegoatTree::resetRebuildStats() {
    stats = RebuildStats{};
}


