//
// Created by laptop on 2024/10/4.
//

#include "LSMController.h"

#include <sys/stat.h>

#include <algorithm>
#include <cstring>
#include <unordered_set>
#include <filesystem>
#include <utility>
#include <unistd.h>

#include "BufferPool.h"
#include "Constants.h"

#include "SSTController.h"

string METADATA_FILENAME_LSM = "metadata";
string SST_FILENAME_LSM = "sst-";
int SIZE_RATIO = 2;

LSMController::LSMController(string theDbName, int bufferPoolCapacity)
        : bufferPool(bufferPoolCapacity), myDbName(std::move(theDbName)) {
    // Step 1: create the directory and metadata if not exist
    if (mkdir(myDbName.c_str(), 0777) == 0) {
        updateMetaData();
        return;
    }

    // Step 2: read the metaData
    readMetaData();
}

int LSMController::updateMetaData() {
    cout << "Storing Metadata..." << endl;
    ofstream outputFile(buildPath(METADATA_FILENAME_LSM));
    if (!outputFile) {
        return -1;
    }

    // Write each key-value pair to the file
    for (const auto &[key, value]: myLevelMap) {
        outputFile << key << " " << value << "\n";
    }

    if (outputFile.fail()) {
        return -1;
    }

    outputFile.close();
    cout << "Metadata Stored." << endl;
    return 0;
}

int LSMController::readMetaData() {
    cout << "Loading Metadata..." << endl;
    ifstream inputFile(buildPath(METADATA_FILENAME_LSM));
    if (!inputFile) {
        return -1;
    }

    int key;
    int value;

    // Read key-value pairs from the file and populate the map
    while (inputFile >> key >> value) {
        myLevelMap[key] = value;
    }

    if (inputFile.fail()) {
        return -1;
    }

    inputFile.close();
    cout << "Metadata Loaded." << endl;
    return 0;
}

void LSMController::deleteFiles() {
    try {
        if (std::filesystem::exists(myDbName)) {
            std::filesystem::remove_all(myDbName);
        } else {
            std::cout << "Directory not found: " << myDbName << std::endl;
        }
    } catch (const std::exception &e) {
        throw runtime_error("error when deleting SSTs");
    }
}

bool LSMController::save(vector<array<int, 2>> theKVPairs, int theLevel) {
    bool isSaved = performSave(theKVPairs, theLevel);

    // do compaction if needed
    if (!isSaved) {
        cerr << "error when performing save for level: " + to_string(theLevel) << endl;
        return false;
    }

    if (myLevelMap[theLevel] == 2) {
        performCompaction();
        invalidateBufferPool();
    }

    return true;
}

bool LSMController::performSave(vector<array<int, 2>> theKVPairs, int theLevel) {
    // insert the KVPairs into the given level
    string pathToSST = newSSTPath(theLevel);
    // create the directory first if it does not exist already
    string pathToDir = pathToSST.substr(0, pathToSST.size() - 6);
    if (access(pathToDir.c_str(), F_OK) == -1) {
        if (mkdir(pathToDir.c_str(), 0777) != 0) {
            cout << "error when creating directory for level: " + to_string(theLevel) << endl;
            return false;
        }
    }
    ofstream outputFile(pathToSST);

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
    // if the first level does not exist yet
    if (myLevelMap.find(theLevel) == myLevelMap.end()) {
        myLevelMap[theLevel] = 1;
    } else {
        myLevelMap[theLevel] += 1;
    }

    return true;
}

vector<array<int, 2>> LSMController::read(int theLevel, int thePageNum, int theSSTNum) {
    // check buffer pool for page first
    vector<array<int, 2>> pageKVPairs =
            bufferPool.getPage(bufferPool.makeLeveledPageId(theLevel, theSSTNum, thePageNum));

    if (!pageKVPairs.empty()) {
        return pageKVPairs;
    }

    // if we reach here, the page is not in buffer pool. So do an I/O to fetch itE
    ifstream inputFile(existingSSTPath(theLevel, theSSTNum), ios::binary);
    if (!inputFile) {

        throw runtime_error("Error when reading SSTs");
    }

    vector<array<int, 2>> kvPairs;
    char buffer[PAGE_SIZE];

    // calculate the position of the target page and seek to it
    size_t offset = (thePageNum - 1) * PAGE_SIZE;
    inputFile.seekg(offset, std::ios::beg);

    // read the page
    inputFile.read(buffer, sizeof(buffer));
    streamsize bytesRead = inputFile.gcount();

    // no more data to read, return the empty pairs
    if (bytesRead <= 0) {
        return kvPairs;
    }

    int numKVPairs = bytesRead / KVPAIR_SIZE;
    vector<array<int, 2>> ioKVPairs(numKVPairs);

    memcpy(ioKVPairs.data(), buffer, bytesRead);
    kvPairs = ioKVPairs;
    bufferPool.putPage(bufferPool.makeLeveledPageId(theLevel, theSSTNum, thePageNum), ioKVPairs);

    inputFile.close();
    return kvPairs;
}

pair<bool, int> LSMController::get(int theKey) {
    for (int level = 1; level <= myLevelMap.size(); level++) {

        // skip the current level if it has no SST
        if (myLevelMap[level] == 0) {
            continue;
        }

        int pageNum = 1;
        while (true) {
            const vector<array<int, 2>> &kvPairs = read(level, pageNum, 1);
            // if there's no more kvPairs, end inner loop and go to the next level
            if (kvPairs.empty()) {
                break;
            }

            int result = searchSST(kvPairs, theKey);
            if (result != -1) {
                pair<bool, int> returnPair(true, kvPairs[result][1]);
                return returnPair;
            }
            pageNum++;
        }
    }

    // Target not found if we reach here
    pair<bool, int> returnPair(false, -1);
    return returnPair;
}

vector<array<int, 2>> LSMController::scan(int theLow, int theHigh) {
    // a hash set for checking whether a key has already been added to result
    unordered_set<int> lookupSet;
    vector<array<int, 2>> result;

    for (int level = 1; level <= myLevelMap.size(); level++) {
        // skip the current level if it has no SST
        if (myLevelMap[level] == 0) {
            continue;
        }

        int pageNum = 1;
        while (true) {
            // there should only be 1 sst in each level
            const vector<array<int, 2>> &kvPairs = read(level, pageNum, 1);
            // if there's no more kvPairs, end inner loop and go to the next level
            if (kvPairs.empty()) {
                break;
            }

            unsigned long size = kvPairs.size();

            // Skip the current SST if nothing is in range
            if (kvPairs[0][0] > theHigh || kvPairs[size - 1][0] < theLow) {
                pageNum++;
                continue;
            }

            // Do a binary search to find the smallest element in range.
            // Note that it will not return -1 since we are already in range
            int targetIdx = searchSSTSmallestLarger(kvPairs, theLow);

            for (int j = targetIdx; j < size; j++) {
                // skip the current SST if value exceeds theHigh
                if (kvPairs[j][0] > theHigh) {
                    break;
                }

                // skip the current element if it is already added in previous
                // iterations
                if (lookupSet.find(kvPairs[j][0]) != lookupSet.end()) {
                    continue;
                }

                // add the pair into result
                result.push_back(kvPairs[j]);
                // add key to lookupSet to prevent duplicates
                lookupSet.insert(kvPairs[j][0]);
            }

            pageNum++;
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

int LSMController::performCompaction() {
    int currentLevel = 1;
    while (currentLevel <= myLevelMap.size()) {

        // skip the current level if less than 2
        if (myLevelMap[currentLevel] < 2) {
            currentLevel++;
            continue;
        }

        int pageNum = 1;
        while (true) {
            // read the 2 SSTs from the current level
            const vector<array<int, 2>> &kvPairs_1 = read(currentLevel, pageNum, 1);
            const vector<array<int, 2>> &kvPairs_2 = read(currentLevel, pageNum, 2);

            // break the loop when no more data to be read from both SSTs
            if (kvPairs_1.empty() && kvPairs_2.empty()) {
                break;
            }

            // buffer for the output sst
            vector<array<int, 2>> outputKVPairs;
            int i = 0;
            int j = 0;

            while (i < kvPairs_1.size() && j < kvPairs_2.size()) {
                // use the pair from memtable if the key equals
                if (kvPairs_1[i][0] == kvPairs_2[j][0]) {
                    // copy the value unless we are at the max level and encountered a tombstone
                    if (!(currentLevel == myLevelMap.size() && kvPairs_2[j][1] == INT32_MIN)) {
                        outputKVPairs.push_back(kvPairs_2[j]);
                    }
                    i++;
                    j++;
                } else if (kvPairs_1[i][0] < kvPairs_2[j][0]) {
                    if (!(currentLevel == myLevelMap.size() && kvPairs_1[i][1] == INT32_MIN)) {
                        outputKVPairs.push_back(kvPairs_1[i]);
                    }
                    i++;
                } else {
                    if (!(currentLevel == myLevelMap.size() && kvPairs_2[j][1] == INT32_MIN)) {
                        outputKVPairs.push_back(kvPairs_2[j]);
                    }
                    j++;
                }
            }

            // add the remaining elements
            while (i < kvPairs_1.size()) {
                if (!(currentLevel == myLevelMap.size() && kvPairs_1[i][1] == INT32_MIN)) {
                    outputKVPairs.push_back(kvPairs_1[i]);
                }
                i++;
            }

            while (j < kvPairs_2.size()) {
                if (!(currentLevel == myLevelMap.size() && kvPairs_2[j][1] == INT32_MIN)) {
                    outputKVPairs.push_back(kvPairs_2[j]);
                }
                j++;
            }

            // write into a new SST file in the next level
            performSave(outputKVPairs, currentLevel + 1);

            pageNum++;
        }

        // remove the old SSTs
        string path = existingSSTPath(currentLevel, 1);
        if (remove(path.c_str()) != 0) {
            throw runtime_error("Error when removing SSTs during compaction");
        }

        path = existingSSTPath(currentLevel, 2);
        if (remove(path.c_str()) != 0) {
            throw runtime_error("Error when removing SSTs during compaction");
        }

        // update the map
        myLevelMap[currentLevel] = 0;

        currentLevel++;
    }

    return 0;
}

string LSMController::buildPath(string theFilePath) {
    return myDbName + "/" + theFilePath;
}

string LSMController::newSSTPath(int theLevel) {
    if (myLevelMap[theLevel] == SIZE_RATIO - 1) {
        // level-{theLevel}/sst-2
        return buildPath("level-" + to_string(theLevel) + "/" + SST_FILENAME_LSM + "2");
    }

    // level-{theLevel}/sst-1
    return buildPath("level-" + to_string(theLevel) + "/" + SST_FILENAME_LSM + "1");
}

string LSMController::existingSSTPath(int theLevel, int theSSTNum) {
    return buildPath("level-" + to_string(theLevel) + "/" + SST_FILENAME_LSM + to_string(theSSTNum));
}

int LSMController::searchSST(const vector<array<int, 2>> &theKVPairs,
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

int LSMController::searchSSTSmallestLarger(
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

bool LSMController::close() {
    if (updateMetaData() == -1) {
        return false;
    }
    return true;
}

int LSMController::invalidateBufferPool() {
    int capacity = bufferPool.getCapacity();
    bufferPool = BufferPool(capacity);
    return 0;
}

unordered_map<int, int> LSMController::getMetadata() {
    return myLevelMap;
}
