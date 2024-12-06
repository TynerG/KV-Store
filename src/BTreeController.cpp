#include "BTreeController.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Constants.h"

using namespace std;

BTreeNodeValue::BTreeNodeValue(int key, int childPage) {
    this->key = key;
    this->childPage = childPage;
}

BTreeNode::BTreeNode(int size, vector<BTreeNodeValue> values) {
    this->size = size;
    this->values = values;
}

BTreeNode::BTreeNode() {
    size = 0;
    values = vector<BTreeNodeValue>();
};

BTreeController::BTreeController(string myDbName,
                                 shared_ptr<SSTController> mySSTController)
    : myDbName(std::move(myDbName)), mySSTController(mySSTController) {}

void BTreeController::createBTrees() {
    int totalSSTs = mySSTController->getMetadata();

    for (int sstIdx = 1; sstIdx <= totalSSTs; sstIdx++) {
        createBTree(sstIdx);
    }
}

string BTreeController::getBTreeFileName(int sstIdx) {
    return getBTreeDirectory() + "/sst-" + to_string(sstIdx) + ".btree";
}

string BTreeController::getBTreeDirectory() { return myDbName + "/btree"; }

bool BTreeController::isBTreeCreated() {
    string btreeDirPath = getBTreeDirectory();
    return access(btreeDirPath.c_str(), F_OK) != -1;
}

bool BTreeController::isNodeLeaf(BTreeNode node) {
    return node.values[0].childPage >= 0;
}

void BTreeController::createBTree(int sstIdx) {
    if (!isBTreeCreated()) {
        string btreeDirPath = getBTreeDirectory();
        if (mkdir(btreeDirPath.c_str(), 0777) != 0) {
            throw runtime_error("Failed to create directory: " + btreeDirPath);
        }
    }

    vector<array<int, 2>> sstData = mySSTController->readSST(sstIdx);

    // open B Tree file
    string btreeFilePath = getBTreeFileName(sstIdx);

    int fd = open(btreeFilePath.c_str(), O_WRONLY | O_CREAT, 0777);
    if (fd < 0) {
        throw runtime_error("Error opening B-tree file for writing: " +
                            btreeFilePath);
    }

    size_t numKVPairsInFile = sstData.size();

    // build the B-tree levels from leaf nodes upwards
    vector<vector<BTreeNode>> BTreeLevels;

    // add the leaf nodes as the first level
    vector<BTreeNode> leafNodes;
    int pageNum = 0;
    vector<BTreeNodeValue> currLeafValues;
    while (true) {
        int remainingKVPairs = numKVPairsInFile - (pageNum * B);
        if (remainingKVPairs <= 0) {
            break;
        }

        int kvPairsInPage = std::min(remainingKVPairs, B);

        // use the last key of the page as node value
        int nodeValue = sstData[pageNum * B + kvPairsInPage - 1][0];

        BTreeNodeValue leafNodeValue = BTreeNodeValue(nodeValue, pageNum);

        pageNum++;
        currLeafValues.push_back(leafNodeValue);

        if (currLeafValues.size() == B) {
            BTreeNode leafNode =
                BTreeNode(currLeafValues.size(), currLeafValues);
            currLeafValues.clear();
            leafNodes.push_back(leafNode);
        }
    }
    // add the remainder
    if (currLeafValues.size() > 0) {
        BTreeNode leafNode = BTreeNode(currLeafValues.size(), currLeafValues);
        currLeafValues.clear();
        leafNodes.push_back(leafNode);
    }

    BTreeLevels.push_back(leafNodes);

    // cout << "leaf values: " << leafNodes[0].values.size() << endl;

    // construct upper level nodes
    while (BTreeLevels.back().size() > 1) {
        vector<BTreeNode> parentNodes;
        vector<BTreeNodeValue> parentNodesValues;
        int totalChildren = BTreeLevels.back().size();

        // group nodes into upper-level parent nodes
        for (int childPage = 0; childPage < totalChildren; childPage += B) {
            int endIdx = std::min(
                childPage + B - 1,
                totalChildren - 1);  // handle last page that may not be full
            int nodeValue =
                BTreeLevels.back()[endIdx]
                    .values.back()
                    .key;  // using the last key of the node as the parent key

            // here we use * -1 to indicate that the childPage is
            // within the next Tree level and not in SST
            int childPageAdjusted =
                -1 * (childPage + 1);  // use 1 based indexing
            BTreeNodeValue parentNode =
                BTreeNodeValue(nodeValue, childPageAdjusted);

            parentNodesValues.push_back(parentNode);
            cout << "parentNodesValues: " << parentNodesValues.size();

            if (parentNodesValues.size() == B) {
                BTreeNode parentNode =
                    BTreeNode(parentNodesValues.size(), parentNodesValues);
                parentNodesValues.clear();
                parentNodes.push_back(parentNode);
            }
        }
        // add the remaining nodes
        if (parentNodesValues.size() > 0) {
            BTreeNode parentNode =
                BTreeNode(parentNodesValues.size(), parentNodesValues);
            parentNode.values = parentNodesValues;
            parentNodesValues.clear();
            parentNodes.push_back(parentNode);
        }
    }

    // cout << "Root node size: " << BTreeLevels.back().size() << endl;
    char buffer[bTreeNodeSize];

    int pagesSoFar = 1;
    // write the B-tree nodes to the file
    for (int levelIdx = BTreeLevels.size() - 1; levelIdx >= 0; levelIdx--) {
        vector<BTreeNode>& level = BTreeLevels[levelIdx];
        for (BTreeNode& node : level) {
            if (!isNodeLeaf(node)) {
                // adjust page position in the current level to account for
                // number of pages before the level
                for (auto& page : node.values) {
                    page.key -= pagesSoFar;
                    pagesSoFar++;
                }
            }

            // reset the buffer for each node
            memset(buffer, 0, sizeof(buffer));

            memcpy(buffer, &node.size, sizeof(int));
            memcpy(buffer + sizeof(int), node.values.data(),
                   node.values.size() * sizeof(BTreeNodeValue));

            ssize_t bytesWritten = write(fd, buffer, bTreeNodeSize);
            if (bytesWritten == -1) {
                close(fd);
                throw runtime_error("Error writing B-tree node to file");
            }

            // cout << "written: " << bytesWritten << endl;
        }
    }

    close(fd);
}

int BTreeController::binarySearchNodeValues(vector<BTreeNodeValue> nodeValues,
                                            int key) {
    int size = nodeValues.size();
    int l = 0;
    int r = size - 1;
    while (l <= r) {
        int mid = l + (r - l) / 2;
        if (nodeValues[mid].key < key) {
            l = mid + 1;
        } else {
            r = mid - 1;
        }
    }

    // cout << "idx: " << l << ", key: " << key << endl;
    // for (auto& value : nodeValues) {
    //     cout << value.key << " ";
    // }
    return nodeValues[l].childPage;
}

BTreeNode BTreeController::readBTreePage(int fd, int page) {
    char buffer[bTreeNodeSize];  // size of 1 I/O

    memset(buffer, 0, sizeof(buffer));  // reset

    // read the first B-tree node (root) into memory
    ssize_t bytesRead = pread(fd, buffer, sizeof(buffer), page * bTreeNodeSize);
    if (bytesRead <= 0) {
        close(fd);
        throw runtime_error("Error reading B-tree file");
    }

    BTreeNode root;
    memcpy(&root.size, buffer + 4, sizeof(int));
    int offset = sizeof(int);  // start from after size is read

    // read the node values from the buffer
    for (int i = 0; i < root.size; i++) {
        int value;
        int childPage;
        memcpy(&value, buffer + offset, sizeof(int));
        memcpy(&childPage, buffer + offset + sizeof(int), sizeof(int));
        // cout << "value: " << value << endl;
        // cout << "childPage: " << childPage << endl;
        root.values.push_back(BTreeNodeValue(value, childPage));
        offset += sizeof(BTreeNodeValue);
    }

    return root;
}

pair<bool, int> BTreeController::searchBTreeNodes(int key, int sstIdx) {
    // construct the path for the B-tree file for this SST file
    string btreeFilePath = getBTreeFileName(sstIdx);

    // open the B-tree file for reading
    int fd = open(btreeFilePath.c_str(), O_RDONLY);
    if (fd < 0) {
        throw runtime_error("Error opening B-tree file for reading: " +
                            btreeFilePath);
    }

    BTreeNode root = readBTreePage(fd, 0);

    BTreeNode curr = root;
    // traverse the B-tree
    while (!isNodeLeaf(curr)) {
        if (curr.values.back().key < key) {
            // the key not found
            return {false, 0};
        }

        // binary search to search for key position
        int childPage = binarySearchNodeValues(curr.values, key);
        childPage = childPage * -1 - 1;  // convert internal node page key

        curr = readBTreePage(fd, childPage);
    }

    // now on a leaf node
    // binary search to search for key position
    int sstPage = binarySearchNodeValues(
        curr.values, key);  // leaf node child page points to sst page

    // finally we read the page in the sst and search it for the key
    auto kvPairs = mySSTController->readSSTPage(sstIdx, sstPage);

    int resIdx = mySSTController->searchSST(kvPairs, key);
    if (resIdx != -1) {
        return {true, kvPairs[resIdx][1]};
    }

    close(fd);

    // the key not found
    return {false, 0};
}

pair<bool, int> BTreeController::get(int key) {
    if (!isBTreeCreated()) {
        throw runtime_error(
            "B-tree is not created. Create it first to use B-tree search.");
    }

    int totalSSTs = mySSTController->getMetadata();

    // search through the B-trees of each SST file (from newest to oldest)
    for (int i = totalSSTs; i > 0; i--) {
        pair<bool, int> result = searchBTreeNodes(key, i);
        if (result.first) {
            return result;
        }
    }

    // not found
    return {false, 0};
}
