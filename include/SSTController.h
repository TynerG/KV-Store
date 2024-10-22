//
// Created by laptop on 2024/10/4.
//

#ifndef AVLTREEPROJECT_SSTCONTROLLER_H
#define AVLTREEPROJECT_SSTCONTROLLER_H

#include <iostream>
#include <fstream>
#include <array>
#include <vector>

#include "BufferPool.h"

using namespace std;

/**
 * Represents and controls the SST part of the KV store.
 */
class SSTController {
private:
    /**
     * BufferPool for SST pages
     */
    BufferPool bufferPool;

    /**
     * The number of SSTs in the database
     */
    int myNumSST;

    /**
     * The name of the database
     */
    string myDbName;

    /**
     * read the metadata from the db
     * @return the number of SSTs, or -1 when no metadata exist
     */
    int readMetaData();

    /**
     * update the metadata with myNumSST
     * @return 0 if success, -1 otherwise
     */
    int updateMetaData();

    /**
     * generate a path to the provided filename using myDbName
     */
    string buildPath(string theFileName);

    /**
     * generate a path for a new SST file
     */
    string newSSTPath();

    /**
     * generate a path for an existing SST file
     * @param theSSTIdx the index of the SST
     */
    string existingSSTPath(int theSSTIdx);

    /**
     * perform a binary search on the given KV-Pairs
     * @param theTarget
     * @return the index of the target, or -1 if target not found
     */
    int searchSST(const vector<array<int, 2>> &theKVPairs, int theTarget);

    /**
     * perform a binary search on the given KV-Pairs and return the smallest element that is larger than or equal to
     * the target
     * @param theTarget
     * @return the index of the smallest element that is larger than or equal to the target, or -1 if target not found
     */
    int searchSSTSmallestLarger(const vector<array<int, 2>> &theKVPairs, int theTarget);

public:

    explicit SSTController(string theDbName, int bufferPoolCapacity);

    /**
     * Get the most up-to-date value of the given key from all SSTs.
     * @return a pair where the first element representing whether the target is found, and the second element
     * representing the value found (or -1 if the first element is false)
     */
    pair<bool, int> get(int theKey);

    /**
     * Retrieves all KV-pairs in a key range in key order (high < low)
     */
    vector<array<int, 2>> scan(int theHigh, int theLow);

    /**
     * Save the given KV-pairs as a SST in the database
     * @param theKVPairs
     * @return whether the save is success
     */
    bool save(vector<array<int, 2>> theKVPairs);

    /**
     * read the given SST file from the database and convert into KV-pairs
     * @param theKVPairs the index of the SST
     * @return the KV-pairs
     *
     * TODO: currently this reads an entire SST at once. Might need to change to read only a page of the file so it
     *  can fit into memory. Let's ask the professor if it is needed.
     */
    vector<array<int, 2>> read(int theSSTIdx);

    /**
     * return the metadata
     * @return number of SSTs
     */
    int getMetadata();
};


#endif //AVLTREEPROJECT_SSTCONTROLLER_H
