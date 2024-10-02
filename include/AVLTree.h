#ifndef AVLTREE_H
#define AVLTREE_H

#include <iostream>
#include <string>

#include <vector>
#include <array>

using namespace std;

class Node {
   public:
    int key;
    int value;
    Node* left;
    Node* right;
    int height;

    Node(int key, int value);
};

class AVLTree {
   public:
    Node* root;
    int capacity;
    int size;

    explicit AVLTree(int capacity);
    // Inserts and returns whether the tree is full
    bool insert(int key, int value);
    // Returns the value of the key
    int getValue(int key);
    // Scan the tree between key 1 and key 2, return a list of key value pairs
    vector<array<int, 2>> scan(Node* node, int low, int high);
    vector<array<int, 2>> scan(int low, int high);
    vector<array<int, 2>> scan(Node* node);
    vector<array<int, 2>> scan();

    // Returns a string representation of the tree
    string inOrderString();
    string preOrderString();

    // Destructor
    ~AVLTree();

   private:
    static int getHeight(Node* node);
    static int getBalance(Node* node);
    Node* insertNode(Node* node, int key, int val);
    int getValueNode(Node* node, int key);
    static Node* rightRotate(Node* node);
    static Node* leftRotate(Node* node);
    string preOrderNodeString(Node* node);
    void deleteTree(Node* node);
};

#endif
