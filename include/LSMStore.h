//
// A class for the KV Store
//

#ifndef AVLTREEPROJECT_LSMSTORE_H
#define AVLTREEPROJECT_LSMTORE_H

#include <memory>

#include "./AVLTree.h"
#include "./LSMController.h"

/**
 * Represents the entire KV store.
 */
class LSMStore {
private:
    int myMemtableSize;

    shared_ptr<AVLTree> myMemtable;

    shared_ptr<LSMController> myLSMController;

    static vector<array<int, 2>> mergeScanResults(
            const vector<array<int, 2>> &scanMem,
            const vector<array<int, 2>> &scanSST);

public:
    /**
     * Constructs a KVStore object with the specified parameters.
     * @param memtableSize The size of the memtable (how many kv pairs)
     * @param dBName The name of the database
     * @param bufferCapacity The maximum number of pages that the buffer pool
     * can hold
     */
    LSMStore(int memtableSize, string dBName, int bufferCapacity);

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
     * Retrieves all KV-pairs in a key range in key order (low < high)
     * @return A vector of KV-Pairs
     */
    vector<array<int, 2>> scan(int low, int high);

    /**
     * Deletes a given KV-pair
     */
    bool remove(int key);

    /**
     * Save everything in memtable into SST and closes the database
     * @return
     */
    bool close();

    /**
     * Delete the database and all its files
     */
    void deleteDb();
};

#endif  // AVLTREEPROJECT_LSMSTORE_H
