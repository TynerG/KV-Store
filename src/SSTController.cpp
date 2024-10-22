//
// Created by laptop on 2024/10/4.
//

#include "SSTController.h"

#include <sys/stat.h>

#include <algorithm>
#include <cstring>
#include <unordered_set>
#include <utility>

#include "BufferPool.h"
#include "Constants.h"

string METADATA_FILENAME = "metadata";
string SST_FILENAME = "sst-";

SSTController::SSTController(string theDbName, int bufferPoolCapacity)
    : bufferPool(bufferPoolCapacity), myDbName(std::move(theDbName)) {
    // Step 1: create the directory and metadata if not exist
    if (mkdir(myDbName.c_str(), 0777) == 0) {
        myNumSST = 0;
        updateMetaData();
        return;
    }

    // Step 2: read the metaData
    readMetaData();
}

int SSTController::updateMetaData() {
    ofstream outputFile(buildPath(METADATA_FILENAME));

    if (!outputFile) {
        return -1;
    }

    outputFile << myNumSST;

    if (outputFile.fail()) {
        return -1;
    }

    outputFile.close();
    return 0;
}

int SSTController::readMetaData() {
    ifstream inputFile(buildPath(METADATA_FILENAME));
    if (!inputFile) {
        return -1;
    }

    inputFile >> myNumSST;

    if (inputFile.fail()) {
        return -1;
    }

    inputFile.close();
    return 0;
}

bool SSTController::save(vector<array<int, 2>> theKVPairs) {
    ofstream outputFile(newSSTPath());

    if (!outputFile) {
        return false;
    }

    outputFile.write(reinterpret_cast<const char *>(theKVPairs.data()),
                     theKVPairs.size() * sizeof(array<int, 2>));

    if (outputFile.fail()) {
        return false;
    }

    outputFile.close();

    // update the metadata
    myNumSST++;
    updateMetaData();

    return true;
}

// vector<array<int, 2>> SSTController::read(int theSSTIdx) {
//     ifstream inputFile(existingSSTPath(theSSTIdx));
//     if (!inputFile) {
//         throw runtime_error("error when reading SSTs");
//     }

//     // Get the size of the file
//     inputFile.seekg(0, ios::end);
//     streamsize sizeSST = inputFile.tellg();
//     inputFile.seekg(0, ios::beg);

//     // Calculate how many KV pairs we need to read. Note that the SSTs might
//     differ in size because we need to save
//     // the memtable when closing the DB no matter if it is full.
//     size_t numKVPairs = sizeSST / sizeof(array<int, 2>);
//     vector<array<int, 2>> kvPairs(numKVPairs);

//     // Read the data
//     if (!inputFile.read(reinterpret_cast<char *>(kvPairs.data()), sizeSST)) {
//         throw runtime_error("error when reading SSTs");
//     }

//     inputFile.close();
//     return kvPairs;
// }

vector<array<int, 2>> SSTController::read(int theSSTIdx) {
    ifstream inputFile(existingSSTPath(theSSTIdx), ios::binary);
    if (!inputFile) {
        throw runtime_error("Error when reading SSTs");
    }

    vector<array<int, 2>> kvPairs;

    char buffer[PAGE_SIZE];
    int pageNum = 0;
    while (inputFile) {
        // read the file page by page
        inputFile.read(buffer, sizeof(buffer));
        streamsize bytesRead = inputFile.gcount();
        if (bytesRead <= 0) break;  // no more data to read

        // check buffer pool for page first
        vector<array<int, 2>> pageKVPairs =
            bufferPool.getPage(theSSTIdx, pageNum);
        if (pageKVPairs.empty()) {
            // not in buffer pool, so do an I/O and add the page to buffer pool
            int numKVPairs = bytesRead / KVPAIR_SIZE;
            vector<array<int, 2>> ioKVPairs(numKVPairs);

            memcpy(ioKVPairs.data(), buffer, bytesRead);
            pageKVPairs = ioKVPairs;
            bufferPool.putPage(theSSTIdx, pageNum, ioKVPairs);
        }

        // add to all
        kvPairs.insert(kvPairs.end(), pageKVPairs.begin(), pageKVPairs.end());

        pageNum++;
    }

    inputFile.close();
    return kvPairs;
}

pair<bool, int> SSTController::get(int theKey) {
    for (int i = myNumSST; i > 0; i--) {
        const vector<array<int, 2>> &sst = read(i);
        int result = searchSST(sst, theKey);

        if (result != -1) {
            pair<bool, int> returnPair(true, sst[result][1]);
            return returnPair;
        }
    }

    // Target not found if we reach here
    pair<bool, int> returnPair(false, -1);
    return returnPair;
}

vector<array<int, 2>> SSTController::scan(int theLow, int theHigh) {
    // a hash set for checking whether a key has already been added to result
    unordered_set<int> lookupSet;
    vector<array<int, 2>> result;

    for (int i = myNumSST; i > 0; i--) {
        const vector<array<int, 2>> &sst = read(i);
        unsigned long size = sst.size();

        // Skip the current SST if nothing is in range
        if (sst[0][1] > theHigh || sst[size - 1][1] < theLow) {
            continue;
        }

        // Do a binary search to find the smallest element in range.
        // Note that it will not return -1 since we are already in range
        int targetIdx = searchSSTSmallestLarger(sst, theLow);

        for (int j = targetIdx; j < size; j++) {
            // skip the current SST if value exceeds theHigh
            if (sst[j][0] > theHigh) {
                break;
            }

            // skip the current element if it is already added in previous
            // iterations
            if (lookupSet.find(sst[j][0]) != lookupSet.end()) {
                continue;
            }

            // add the pair into result
            result.push_back(sst[j]);
            // add key to lookupSet to prevent duplicates
            lookupSet.insert(sst[j][0]);
        }
    }

    // sort the result based on key
    // TODO: any ways to avoid the sort?
    sort(result.begin(), result.end(),
         [](const array<int, 2> &a, const array<int, 2> b) {
             return a[0] < b[0];
         });

    return result;
}

int SSTController::getMetadata() { return myNumSST; }

string SSTController::buildPath(string theFileName) {
    return myDbName + "/" + theFileName;
}

string SSTController::newSSTPath() {
    return buildPath(SST_FILENAME + to_string(myNumSST + 1));
}

string SSTController::existingSSTPath(int theSSTIdx) {
    return buildPath(SST_FILENAME + to_string(theSSTIdx));
}

int SSTController::searchSST(const vector<array<int, 2>> &theKVPairs,
                             int theTarget) {
    int lowIdx = 0;
    int highIdx = theKVPairs.size() - 1;

    while (lowIdx <= highIdx) {
        int midIdx = lowIdx + (highIdx - lowIdx) / 2;

        // Check if the target is found
        if (theKVPairs[midIdx][0] == theTarget) {
            return midIdx;
        }

        // Update the middle index according to the value of the current element
        if (theKVPairs[midIdx][0] < theTarget) {
            lowIdx = midIdx + 1;
        } else {
            highIdx = midIdx - 1;
        }
    }

    // Target not found
    return -1;
}

int SSTController::searchSSTSmallestLarger(
    const vector<array<int, 2>> &theKVPairs, int theTarget) {
    int lowIdx = 0;
    int highIdx = theKVPairs.size() - 1;
    int resultIdx = -1;

    while (lowIdx <= highIdx) {
        int midIdx = lowIdx + (highIdx - lowIdx) / 2;

        // Check if the target is found
        if (theKVPairs[midIdx][0] == theTarget) {
            return midIdx;
        }

        // Update the middle index according to the value of the current
        // element, along with the potential result
        if (theKVPairs[midIdx][0] < theTarget) {
            lowIdx = midIdx + 1;
        } else {
            highIdx = midIdx - 1;
            // record potential result
            resultIdx = midIdx;
        }
    }

    // Target not found
    return resultIdx;
}
