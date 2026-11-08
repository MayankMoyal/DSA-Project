#include "scapegoat_tree.h"
#include <iostream>
using namespace std;
typedef long long ll;


ScapegoatTree::ScapegoatTree() {
    root = nullptr;
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
    root = insertNode(root, key);
}


bool ScapegoatTree::contains(ll key) const {
    Node* cur = root;
    while (cur != nullptr) {
        if (key < cur->key) {
            cur = cur->left;
        }

        else if (key > cur->key) {
            cur = cur->right;
        } 
        
        else {
            return true;
        }
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
    cout<<'\n';
}

int ScapegoatTree::heightOf(Node* v) const {

    if (v == nullptr) return -1;

    int leftHeight = heightOf(v->left);
    int rightHeight = heightOf(v->right);

    if (leftHeight > rightHeight) return leftHeight + 1;

    return rightHeight + 1;
}

int ScapegoatTree::height() const {
    return heightOf(root);
}







