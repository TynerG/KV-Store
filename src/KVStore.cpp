//
// Created by laptop on 2024/10/4.
//

#include <cstring>
#include "KVStore.h"

KVStore::KVStore(int memtableSize, string dBName, int bufferCapacity) {
    myMemtableSize = memtableSize;
    myMemtable = make_shared<AVLTree>(memtableSize);
    mySSTController = make_shared<SSTController>(dBName, bufferCapacity);
}

bool KVStore::put(int key, int value) {
    bool result = myMemtable->insert(key, value);

    // flush data to SST if the memtable is full
    if (result) {
        const vector<array<int, 2>> &buffer = myMemtable->scan();
        mySSTController->save(buffer);

        // clean the memtable by creating a new tree
        myMemtable = make_shared<AVLTree>(myMemtableSize);
        return put(key, value);
    }

    return true;
}

int KVStore::get(int key) {
    int result;

    try {
        result = myMemtable->getValue(key);
    } catch (const exception &e) {
        // if not found in memtable, search the SSTs instead
        if (strcmp(e.what(), "Key not found") == 0) {
            const pair<bool, int> &pair = mySSTController->get(key);

            if (pair.first) {
                result = pair.second;
            } else {
                throw std::runtime_error("Key not found");
            }
        }
    }

    return result;
}

vector<array<int, 2>> KVStore::scan(int low, int high) {
    const vector<array<int, 2>> &scanMem = myMemtable->scan(low, high);
    const vector<array<int, 2>> &scanSST = mySSTController->scan(low, high);

    // merge the results
    const vector<array<int, 2>> &results = mergeScanResults(scanMem, scanSST);

    return results;
}

vector<array<int, 2>>
KVStore::mergeScanResults(const vector<array<int, 2>> &scanMem, const vector<array<int, 2>> &scanSST) {
    vector<array<int, 2>> result;
    int i = 0;
    int j = 0;

    while (i < scanMem.size() && j < scanSST.size()) {
        // use the pair from memtable if the key equals
        if (scanMem[i][0] == scanSST[j][0]) {
            result.push_back(scanMem[i]);
            i++;
            j++;
        } else if (scanMem[i][0] < scanSST[j][0]) {
            result.push_back(scanMem[i]);
            i++;
        } else {
            result.push_back(scanSST[j]);
            j++;
        }
    }

    // add the remaining elements
    while (i < scanMem.size()) {
        result.push_back(scanMem[i]);
        i++;
    }

    while (j < scanSST.size()) {
        result.push_back(scanSST[j]);
        j++;
    }

    return result;
}

bool KVStore::close() {
    // store the memtable into SST
    return mySSTController->save(myMemtable->scan());
}
