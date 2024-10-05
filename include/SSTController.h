//
// Created by laptop on 2024/10/4.
//

#ifndef AVLTREEPROJECT_SSTCONTROLLER_H
#define AVLTREEPROJECT_SSTCONTROLLER_H

#include <iostream>
#include <fstream>
#include <array>
#include <vector>

using namespace std;

class SSTController {
private:

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

public:

    explicit SSTController(string theDbName);

    /**
     * Get the most up-to-date value of the given key.
     */
    int get(int theKey);

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
     */
    vector<array<int, 2>> read(int theSSTIdx);

    /**
     * return the metadata
     * @return number of SSTs
     */
    int getMetadata();
};


#endif //AVLTREEPROJECT_SSTCONTROLLER_H
