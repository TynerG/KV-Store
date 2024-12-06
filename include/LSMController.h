//
// Created by laptop on 2024/10/4.
//

#ifndef AVLTREEPROJECT_LSMCONTROLLER_H
#define AVLTREEPROJECT_LSMCONTROLLER_H

#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <unordered_map>

#include "BufferPool.h"

using namespace std;

/**
 * Represents and controls the SST part of the KV store.
 */
class LSMController {
private:
    /**
     * BufferPool for SST pages
     */
    BufferPool bufferPool;

    /**
     * A map of level to the number of SSTs it contains.
     */
    unordered_map<int, int> myLevelMap;

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
    string buildPath(string theFilePath);

    /**
     * generate a path for an SST file at a given level
     */
    string newSSTPath(int theLevel);

    /**
     * generate a path for an existing SST file
     * @param theLevel the number of the level
     * @param theSSTNum the number of the SST
     */
    string existingSSTPath(int theLevel, int theSSTNum);

    /**
     * read the given page of the given SST file from the database and convert into KV-pairs
     * @param theLevel the level of the SST
     * @param thePageNum the target page of the SST
     * @param theSSTNum the index of the SST
     * @return the KV-pairs
     */
    vector<array<int, 2>> read(int theLevel, int thePageNum, int theSSTNum);

    /**
     * perform a binary search on the given KV-Pairs
     * @param theTarget
     * @return the index of the target, or -1 if target not found
     */
    static int searchSST(const vector<array<int, 2>> &theKVPairs, int theTarget);

    /**
     * perform a binary search on the given KV-Pairs and return the smallest element that is larger than or equal to
     * the target
     * @param theTarget
     * @return the index of the smallest element that is larger than or equal to the target, or -1 if target not found
     */
    static int searchSSTSmallestLarger(const vector<array<int, 2>> &theKVPairs, int theTarget);

    /**
     * perform compaction in the DB
     */
    int performCompaction();

    /**
     * Save the sst without triggering compaction
     * @param theKVPairs the KV-pairs to be saved.
     * @param theLevel the level to be inserted.
     * @return whether the save is success
     */
    bool performSave(vector<array<int, 2>> theKVPairs, int theLevel);

    /**
     * Invalidate all pages in the buffer pool
     * @return whether the save is success
     */
    int invalidateBufferPool();

public:

    explicit LSMController(string theDbName, int bufferPoolCapacity);

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
     * Save the given KV-pairs as a SST to a given level in the database
     * @param theKVPairs
     * @return whether the save is success
     */
    bool save(vector<array<int, 2>> theKVPairs, int theLevel);

    /**
     * deletes the sst files
     * @return whether the delete was successful
     */
    void deleteFiles();

    /**
     * Return the level map
     */
     unordered_map<int, int> getMetadata();

    /**
     * close the LSM tree and store the metadata
     */
    bool close();
};


#endif //AVLTREEPROJECT_LSMCONTROLLER_H
