#ifndef AVLTREE_H
#define AVLTREE_H

#include <iostream>
#include <string>

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

    AVLTree(int capacity);
    // Inserts and returns whether the tree is full
    bool insert(int key, int value);
    // Returns the value of the key
    int getValue(int key);
    
    // Returns a string representation of the tree
    std::string inOrder();
    std::string preOrder();

    // Destructor
    ~AVLTree();

   private:
    int getHeight(Node* node);
    int getBalance(Node* node);
    Node* insertNode(Node* node, int key, int val);
    int getValueNode(Node* node, int key);
    Node* rightRotate(Node* node);
    Node* leftRotate(Node* node);
    std::string inOrderNode(Node* node);
    std::string preOrderNode(Node* node);
    void deleteTree(Node* node);
};

#endif
