#include "../include/AVLTree.h"

using namespace std;

Node::Node(int key, int value)
        : key(key), value(value), left(nullptr), right(nullptr), height(1) {}

AVLTree::AVLTree(int capacity) : root(nullptr), capacity(capacity), size(0) {}

string AVLTree::inOrderString() {
    vector <array<int, 2>> scanResult = scan(this->root);

    string result;
    for (auto pair: scanResult){
        result += to_string(pair[0]) + " ";
    }

    return result;
}

int AVLTree::getValue(int key) { return getValueNode(this->root, key); }

int AVLTree::getValueNode(Node *node, int key) {
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

string AVLTree::preOrderString() { return preOrderNodeString(this->root); }

string AVLTree::preOrderNodeString(Node *node) {
    if (node == nullptr) {
        return "";
    }

    string result = to_string(node->key) + " ";
    result += preOrderNodeString(node->left);
    result += preOrderNodeString(node->right);
    return result;
}

int AVLTree::getHeight(Node *node) { return node ? node->height : 0; }

int AVLTree::getBalance(Node *node) {
    if (node == nullptr) {
        return 0;
    }
    return getHeight(node->left) - getHeight(node->right);
}

vector <array<int, 2>> AVLTree::scan(int low, int high) {
    return scan(root, low, high);
}

vector <array<int, 2>> AVLTree::scan(Node *node, int low, int high) {
    if (node == nullptr) {
        return {};
    }

    vector <array<int, 2>> result;
    if (node->key > low) {
        // scan the left subtree
        result = scan(node->left, low, high);
    }

    if (high >= node->key && node->key >= low) {
        // current node
        result.push_back({node->key, node->value});
    }

    if (node->key < high) {
        // scan the right subtree
        vector <array<int, 2>> right_result = scan(node->right, low, high);
        result.insert(result.end(), right_result.begin(), right_result.end());
    }

    return result;
}

vector <array<int, 2>> AVLTree::scan() {
    return scan(root);
}

vector <array<int, 2>> AVLTree::scan(Node *node) {
    if (node == nullptr) {
        return {};
    }

    vector <array<int, 2>> result;
    // scan the left subtree
    result = scan(node->left);
    // current node
    result.push_back({node->key, node->value});
    // scan the right subtree
    vector <array<int, 2>> right_result = scan(node->right);
    result.insert(result.end(), right_result.begin(), right_result.end());

    return result;
}

bool AVLTree::insert(int key, int value) {
    if (this->size == this->capacity) {
        cout << "Cannot insert, tree is full!" << endl;
        return true;
    }

    Node *new_root = insertNode(this->root, key, value);
    this->root = new_root;
    this->size++;
    return this->size >= this->capacity;
}

Node *AVLTree::insertNode(Node *curr, int key, int val) {
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

Node *AVLTree::rightRotate(Node *node) {
    Node *left = node->left;
    Node *left_right = left->right;

    left->right = node;
    node->left = left_right;

    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    left->height = 1 + max(getHeight(left->left), getHeight(left->right));

    return left;
}

Node *AVLTree::leftRotate(Node *node) {
    Node *right = node->right;
    Node *right_left = right->left;

    right->left = node;
    node->right = right_left;

    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    right->height = 1 + max(getHeight(right->left), getHeight(right->right));

    return right;
}

AVLTree::~AVLTree() { deleteTree(root); }

void AVLTree::deleteTree(Node *node) {
    if (node != nullptr) {
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }
}