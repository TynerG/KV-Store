#ifndef BTREECONTROLLER_H
#define BTREECONTROLLER_H

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "BufferPool.h"
#include "Constants.h"
#include "SSTController.h"

using namespace std;

// B tree node value that takes up the same space as KV pair, so 'B' B tree
// nodes per page (node)
struct BTreeNodeValue {
    int key;  // The key for this node value to guide search, the child page of
              // this node value will have values above the previous node value
              // up to the current key value.
    /**
     * If leaf node, this is positive and represents the page within the SST
     that the key associates to, otherwise it's negative and represents the B
     tree node descendant page within the B tree
    */
    int childPage;

    BTreeNodeValue(int key, int childPage);
};

struct BTreeNode {
    int size;                       // Tracks the number of values in this node
    vector<BTreeNodeValue> values;  // Stores at most B values

    BTreeNode(int size, vector<BTreeNodeValue> values);

    BTreeNode();
};

/**
 * Represents and controls the static B tree of each SST file.
 */
class BTreeController {
   private:
    // The name of the database
    string myDbName;

    // Node size in bytes
    const size_t bTreeNodeSize = sizeof(int) + PAGE_SIZE;

    shared_ptr<SSTController> mySSTController;

    /**
     * Searches an SST node file for the page number the key is in
     * @return the page number the key might be in within the SST number
     */
    pair<bool, int> searchBTreeNodes(int key, int sstNum);

    // Creates a new B tree file for the specified SST file
    void createBTree(int sstIdx);

    // Returns the page number of the page containing the key
    int binarySearchNodeValues(vector<BTreeNodeValue> nodeValues, int key);

    // Returns the B-tree node stored in the page position
    BTreeNode readBTreePage(int fd, int page);

    // Returns the path for B-tree nodes file for the SST index
    string getBTreeFileName(int sstIdx);

    // Returns the directory path for B-tree nodes files
    string getBTreeDirectory();

    // Retrurns whether the B-trees are created for the SSTs
    bool isBTreeCreated();

    bool isNodeLeaf(BTreeNode node);

   public:
    BTreeController(string myDbName, shared_ptr<SSTController> mySSTController);

    // For each SST file, create a new file storing its B Tree nodes
    void createBTrees();

    /**
     * Search using for key in the database using SST
     * @return the value associated with the key, or -1 if not found
     */
    pair<bool, int> get(int key);
};

#endif