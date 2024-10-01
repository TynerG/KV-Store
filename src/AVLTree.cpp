#include "../include/AVLTree.h"

#include <iostream>
#include <string>

using namespace std;

Node::Node(int key, int value)
    : key(key), value(value), left(nullptr), right(nullptr), height(1) {}

AVLTree::AVLTree(int capacity) : root(nullptr), capacity(capacity), size(0) {}

string AVLTree::inOrder() { return inOrderNode(this->root); }

string AVLTree::inOrderNode(Node* node) {
    if (node == nullptr) {
        return "";
    }

    string result = inOrderNode(node->left);
    result += to_string(node->key) + " ";
    result += inOrderNode(node->right);
    return result;
}

int AVLTree::getValue(int key) { return getValueNode(this->root, key); }
int AVLTree::getValueNode(Node* node, int key) {
    if (node == nullptr) {
        throw std::runtime_error("Key not found");
    }
    if (key == node->key) {
        return node->value;
    } else if (key < node->key) {
        return getValueNode(node->left, key);
    } else {
        return getValueNode(node->right, key);
    }
}

string AVLTree::preOrder() { return preOrderNode(this->root); }

string AVLTree::preOrderNode(Node* node) {
    if (node == nullptr) {
        return "";
    }

    string result = to_string(node->key) + " ";
    result += preOrderNode(node->left);
    result += preOrderNode(node->right);
    return result;
}

int AVLTree::getHeight(Node* node) { return node ? node->height : 0; }

int AVLTree::getBalance(Node* node) {
    if (node == nullptr) {
        return 0;
    }
    return getHeight(node->left) - getHeight(node->right);
}

bool AVLTree::insert(int key, int value) {
    if (this->size == this->capacity) {
        cout << "Cannot insert, tree is full!" << endl;
        return true;
    }

    Node* new_root = insertNode(this->root, key, value);
    this->root = new_root;
    this->size++;
    return this->size >= this->capacity;
}

Node* AVLTree::insertNode(Node* curr, int key, int val) {
    if (curr == nullptr) return new Node(key, val);

    if (key < curr->key) {
        curr->left = insertNode(curr->left, key, val);
    } else if (key > curr->key) {
        curr->right = insertNode(curr->right, key, val);
    } else {
        cout << "Duplicate key!" << endl;
        curr->value = val;
        return curr;
    }

    curr->height = 1 + max(getHeight(curr->left), getHeight(curr->right));

    int balance = getBalance(curr);
    if (balance > 1 && key < curr->left->key) {
        return rightRotate(curr);
    }
    if (balance > 1 && key > curr->left->key) {
        curr->left = leftRotate(curr->left);
        return rightRotate(curr);
    }
    if (balance < -1 && key > curr->right->key) {
        return leftRotate(curr);
    }
    if (balance < -1 && key < curr->right->key) {
        curr->right = rightRotate(curr->right);
        return leftRotate(curr);
    }

    return curr;
}

Node* AVLTree::rightRotate(Node* node) {
    Node* left = node->left;
    Node* left_right = left->right;

    left->right = node;
    node->left = left_right;

    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    left->height = 1 + max(getHeight(left->left), getHeight(left->right));

    return left;
}

Node* AVLTree::leftRotate(Node* node) {
    Node* right = node->right;
    Node* right_left = right->left;

    right->left = node;
    node->right = right_left;

    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    right->height = 1 + max(getHeight(right->left), getHeight(right->right));

    return right;
}

AVLTree::~AVLTree() { deleteTree(root); }

void AVLTree::deleteTree(Node* node) {
    if (node != nullptr) {
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }
}