//
// Created by laptop on 2024/10/4.
//

#include <sys/stat.h>
#include <cstring>
#include <utility>

#include "SSTController.h"

string METADATA_FILENAME = "metadata";
string SST_FILENAME = "sst-";

SSTController::SSTController(string theDbName) {
    myDbName = std::move(theDbName);

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

    outputFile.write(reinterpret_cast<const char *>(theKVPairs.data()), theKVPairs.size() * sizeof(array<int, 2>));

    if (outputFile.fail()) {
        return false;
    }

    outputFile.close();

    // update the metadata
    myNumSST++;
    updateMetaData();

    return true;
}

vector<array<int, 2>> SSTController::read(int theSSTIdx) {
    ifstream inputFile(existingSSTPath(theSSTIdx));
    if (!inputFile) {
        throw runtime_error("error when reading SSTs");
    }

    // Get the size of the file
    inputFile.seekg(0, ios::end);
    streamsize sizeSST = inputFile.tellg();
    inputFile.seekg(0, ios::beg);

    // Calculate how many KV pairs we need to read. Note that the SSTs might differ in size because we need to save
    // the memtable when closing the DB no matter if it is full.
    size_t numKVPairs = sizeSST / sizeof(array<int, 2>);
    vector<array<int, 2>> kvPairs(numKVPairs);

    // Read the data
    if (!inputFile.read(reinterpret_cast<char *>(kvPairs.data()), sizeSST)) {
        throw runtime_error("error when reading SSTs");
    }

    inputFile.close();
    return kvPairs;
}

int SSTController::getMetadata() {
    return myNumSST;
}

string SSTController::buildPath(string theFileName) {
    return myDbName + "/" + theFileName;
}

string SSTController::newSSTPath() {
    return buildPath(SST_FILENAME + to_string(myNumSST + 1));
}

string SSTController::existingSSTPath(int theSSTIdx) {
    return buildPath(SST_FILENAME + to_string(theSSTIdx));
}


