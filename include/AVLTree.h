#ifndef AVLTREE_H
#define AVLTREE_H

#include <iostream>
#include <string>

#include <vector>
#include <array>

using namespace std;

/**
 * Represents a node in the AVL tree.
 */
class Node {
public:
    int key;
    int value;
    Node *left;
    Node *right;
    int height;

    Node(int key, int value);
};

/**
 * An AVL tree representing the memtable of the KV store.
 */
class AVLTree {
public:
    Node *root;
    int capacity;
    int size;

    explicit AVLTree(int capacity);

    /**
     * Insert the KV-Pair and returns whether the tree is full
     * @return 0 if successfully inserted, 1 if the tree is full
     */
    bool insert(int key, int value);

    /**
     * Returns the value of the key. Throws exception when the key is not found.
     */
    int getValue(int key);

    /**
     * Scan the memtable and retrieves all KV-Pairs in a key range in key order (low < high)
     * @param node the root node of the subtree that will be searched
     * @return a vector of KV-pairs
     */
    vector<array<int, 2>> scan(Node *node, int low, int high);

    /**
     * An overload of scan which defaults to the root node of the AVL tree.
     */
    vector<array<int, 2>> scan(int low, int high);

    /**
     * An overload of scan which defaults scan all the nodes of the tree.
     */
    vector<array<int, 2>> scan(Node *node);

    /**
     * An overload of scan which defaults to the root node of the AVL tree and scan all the nodes.
     */
    vector<array<int, 2>> scan();

    /**
     * Returns a in-order string representation of the tree
     */
    string inOrderString();

    /**
     * Returns a pre-order string representation of the tree
     */
    string preOrderString();

    // Destructor
    ~AVLTree();

private:
    /**
     * get the height of the subtree from the given node
     */
    static int getHeight(Node *node);

    /**
     * get the balance of the subtree from the given node
     */
    static int getBalance(Node *node);

    /**
     * insert a KV-Pair into the subtree from the subtree of the given node
     */
    Node *insertNode(Node *node, int key, int val);

    /**
     * get node with the given key from the subtree of the given node
     */
    int getValueNode(Node *node, int key);

    /**
     * perform a right rotation on the subtree of the given node
     */
    static Node *rightRotate(Node *node);

    /**
     * perform a left rotation on the subtree of the given node
     */
    static Node *leftRotate(Node *node);

    /**
     * a helper for the pre-order string representation of the tree
     */
    string preOrderNodeString(Node *node);

    /**
     * delete the tree and free all memories
     */
    void deleteTree(Node *node);
};

#endif
