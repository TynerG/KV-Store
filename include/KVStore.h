//
// A class for the KV Store
//

#ifndef AVLTREEPROJECT_KVSTORE_H
#define AVLTREEPROJECT_KVSTORE_H

#include <memory>
#include "./AVLTree.h"
#include "./SSTController.h"

/**
 * Represents the entire KV store.
 */
class KVStore {
private:
    int myMemtableSize;

    shared_ptr<AVLTree> myMemtable;

    shared_ptr<SSTController> mySSTController;

    vector<array<int, 2>> mergeScanResults(const vector<array<int, 2>> &scanMem, const vector<array<int, 2>> &scanSST);

public:
    KVStore(int memtableSize, string dBName);

    /**
     * Stores a key associated with a value
     * @return whether the KV-Pair is successfully inserted
     */
    bool put(int key, int value);

    /**
     * Retrieves a value associated with a given key
     * @return
     */
    int get(int key);

    /**
     * retrieves all KV-pairs in a key range in key order (low < high)
     * @return A vector of KV-Pairs
     */
    vector<array<int, 2>> scan(int low, int high);

    /**
     * Save everything in memtable into SST and closes the database
     * @return
     */
    bool close();
};


#endif //AVLTREEPROJECT_KVSTORE_H
