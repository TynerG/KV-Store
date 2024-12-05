//
// Created by laptop on 2024/10/4.
//

#include "LSMStore.h"

#include <cstring>

LSMStore::LSMStore(int memtableSize, string dBName, int bufferCapacity) {
    myMemtableSize = memtableSize;
    myMemtable = make_shared<AVLTree>(memtableSize);
    myLSMController = make_shared<LSMController>(dBName, bufferCapacity);
}

void LSMStore::deleteDb() {
    myLSMController->deleteFiles();
    myMemtable.reset();
}

bool LSMStore::put(int key, int value) {
    bool result = myMemtable->insert(key, value);

    // flush data to SST if the memtable is full
    if (result) {
        const vector<array<int, 2>> &buffer = myMemtable->scan();
        // save to the first level by default
        bool isSaved = myLSMController->save(buffer, 1);
        if (!isSaved) {
            cerr << "could not save memtable" << endl;
            return false;
        }

        // clean the memtable by creating a new tree
        myMemtable = make_shared<AVLTree>(myMemtableSize);
        return put(key, value);
    }

    return true;
}

int LSMStore::get(int key) {
    int result;

    try {
        result = myMemtable->getValue(key);
    } catch (const exception &e) {
        // if not found in memtable, search the SSTs instead
        if (strcmp(e.what(), "Key not found") == 0) {
            const pair<bool, int> &pair = myLSMController->get(key);

            if (pair.first) {
                result = pair.second;
            } else {
                throw std::runtime_error("Key not found");
            }
        }
    }

    // if it is a tombstone
    if (result == INT32_MIN) {
        throw std::runtime_error("Key not found");
    }

    return result;
}

vector<array<int, 2>> LSMStore::scan(int low, int high) {
    const vector<array<int, 2>> &scanMem = myMemtable->scan(low, high);
    const vector<array<int, 2>> &scanSST = myLSMController->scan(low, high);

    // merge the results
    const vector<array<int, 2>> &results = mergeScanResults(scanMem, scanSST);

    return results;
}

vector<array<int, 2>> LSMStore::mergeScanResults(
        const vector<array<int, 2>> &scanMem,
        const vector<array<int, 2>> &scanSST) {
    vector<array<int, 2>> result;
    int i = 0;
    int j = 0;

    while (i < scanMem.size() && j < scanSST.size()) {
        // use the pair from memtable if the key equals
        if (scanMem[i][0] == scanSST[j][0]) {
            // only insert it when it is not a tombstone
            if (scanMem[i][1] != INT32_MIN) {
                result.push_back(scanMem[i]);
            }
            i++;
            j++;
        } else if (scanMem[i][0] < scanSST[j][0]) {
            if (scanMem[i][1] != INT32_MIN) {
                result.push_back(scanMem[i]);
            }
            i++;
        } else {
            if (scanMem[i][1] != INT32_MIN) {
                result.push_back(scanSST[j]);
            }
            j++;
        }
    }

    // add the remaining elements
    while (i < scanMem.size()) {
        if (scanMem[i][1] != INT32_MIN) {
            result.push_back(scanMem[i]);
        }
        i++;
    }

    while (j < scanSST.size()) {
        if (scanSST[j][1] != INT32_MIN) {
            result.push_back(scanSST[j]);
        }
        j++;
    }

    return result;
}

bool LSMStore::remove(int key) {
    // put a tombstone for the given Key
    return put(key, INT32_MIN);
}

bool LSMStore::close() {
    // store the memtable into SST
    cout << "Storing In-memory Data..." << endl;
    bool isClosed = myLSMController->save(myMemtable->scan(), 1);
    myLSMController->close();
    return isClosed;
}
