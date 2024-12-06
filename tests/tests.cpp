#include <sys/stat.h>

#include <cassert>
#include <iostream>

#include "../include/AVLTree.h"
#include "../include/KVStore.h"
#include "../include/SSTController.h"
#include "../include/xxHash32.h"

int readIntFromPath(const string &expectedMetaDataPath);

using namespace std;

// Function template to compare results of generic types
template <typename T>
void checkTestResult(const T &expected, const T &result, int &passed,
                     int &failed) {
    if (result == expected) {
        passed++;
        cout << "--[Passed]" << endl;
    } else {
        failed++;
        cout << "--[Failed]" << endl;
        cout << "Expected '" << expected << "', got '" << result << "'" << endl;
    }
}

string stringifyKvPairs(const vector<array<int, 2>> &the_pair) {
    string result;
    for (auto pair : the_pair) {
        result += "(" + to_string(pair[0]) + "," + to_string(pair[1]) + ") ";
    }

    return result;
}

array<int, 2> runAVLTreeTests() {
    cout << "\n" << endl;
    cout << "#################################" << endl;
    cout << "# Running [AVL tree tests]..." << endl;
    cout << "#################################" << endl;

    // Setup
    int passed = 0;
    int failed = 0;
    AVLTree tree(8);  // Assuming 8 is the initial capacity or some parameter
    string result;

    cout << "Test: Single insertion" << endl;
    tree.insert(10, 10);
    result = tree.preOrderString();
    checkTestResult<string>("10 ", result, passed, failed);

    cout << "Test: Multiple insertions" << endl;
    tree.insert(20, 20);
    tree.insert(30, 30);
    tree.insert(40, 40);
    tree.insert(50, 50);
    tree.insert(25, 123);
    result = tree.preOrderString();
    checkTestResult<string>("30 20 10 25 40 50 ", result, passed, failed);
    result = tree.inOrderString();
    checkTestResult<string>("10 20 25 30 40 50 ", result, passed, failed);

    cout << "Test: Get" << endl;
    int intResult = tree.getValue(25);
    checkTestResult<int>(123, intResult, passed, failed);

    cout << "Test: Insertion limit" << endl;
    bool boolResult = tree.insert(60, 60);
    checkTestResult<bool>(false, boolResult, passed, failed);
    boolResult = tree.insert(70, 60);
    checkTestResult<bool>(true, boolResult, passed, failed);

    cout << "Test: Scan" << endl;
    string expected = "(25,123) (30,30) (40,40) (50,50) (60,60) ";
    checkTestResult(expected, stringifyKvPairs(tree.scan(23, 69)), passed,
                    failed);

    cout << "Test: Scan Inclusive" << endl;
    expected = "(20,20) (25,123) (30,30) (40,40) (50,50) (60,60) (70,60) ";
    checkTestResult(expected, stringifyKvPairs(tree.scan(20, 70)), passed,
                    failed);

    // Summary of tests completed
    cout << "Tests completed: " << passed << "/" << (passed + failed)
         << " passed." << endl;

    array<int, 2> passFail = {passed, failed};
    return passFail;
}

array<int, 2> runSSTControllerTests() {
    cout << "\n" << endl;
    cout << "#################################" << endl;
    cout << "# Running SST Controller tests..." << endl;
    cout << "#################################" << endl;

    // Setup
    int passed = 0;
    int failed = 0;
    int bufferPoolCapacity = 1;

    // build KV-Pairs
    AVLTree tree(8);  // Assuming 8 is the initial capacity or some parameter
    tree.insert(10, 10);
    tree.insert(20, 20);
    tree.insert(50, 50);
    tree.insert(23, 123);
    tree.insert(25, 125);
    tree.insert(24, 124);
    tree.insert(30, 30);
    tree.insert(40, 40);
    const vector<array<int, 2>> &kvPairs = tree.scan();

    // init the controller
    const string dbName = "MyDatabase";
    SSTController controller(dbName, bufferPoolCapacity);

    cout << "Test: Database Creation - Directory Created" << endl;
    string expectedDbPath = "./MyDatabase";
    struct stat info;
    bool actual = stat(expectedDbPath.c_str(), &info) == 0;
    actual = actual && (info.st_mode & S_IFDIR);
    checkTestResult<bool>(true, actual, passed, failed);

    cout << "Test: Database Creation - Metadata Created" << endl;
    string expectedMetaDataPath = "./MyDatabase/metadata";
    int result = readIntFromPath(expectedMetaDataPath);
    checkTestResult<int>(0, result, passed, failed);

    cout << "Test: Saving a KV-Pair as SST" << endl;
    controller.save(kvPairs);
    const vector<array<int, 2>> &savedKvPairs = controller.readSST(1);
    string expectedKvPairs =
        "(10,10) (20,20) (23,123) (24,124) (25,125) (30,30) (40,40) (50,50) ";
    checkTestResult<string>(expectedKvPairs, stringifyKvPairs(savedKvPairs),
                            passed, failed);

    cout << "Test: Maintains Metadata" << endl;
    int expectedMetadata = 1;
    int actualMetadata = readIntFromPath(expectedMetaDataPath);
    checkTestResult<int>(expectedMetadata, actualMetadata, passed, failed);

    cout << "Test: Open Existing Database" << endl;
    expectedMetadata = 1;
    SSTController controllerForExistingDb(dbName, bufferPoolCapacity);
    actualMetadata = controllerForExistingDb.getMetadata();
    checkTestResult<int>(expectedMetadata, actualMetadata, passed, failed);

    cout << "Test: Multiple SSTs" << endl;
    // build another SST
    AVLTree tree2(8);  // Assuming 8 is the initial capacity or some parameter
    tree2.insert(10, 15);
    tree2.insert(20, 25);
    tree2.insert(30, 35);
    tree2.insert(40, 45);
    tree2.insert(50, 55);
    tree2.insert(60, 65);
    tree2.insert(70, 75);
    tree2.insert(80, 85);
    const vector<array<int, 2>> &kvPairs2 = tree2.scan();
    controller.save(kvPairs2);

    const vector<array<int, 2>> &savedKvPairs2 = controller.readSST(2);
    expectedKvPairs =
        "(10,15) (20,25) (30,35) (40,45) (50,55) (60,65) (70,75) (80,85) ";
    checkTestResult<string>(expectedKvPairs, stringifyKvPairs(savedKvPairs2),
                            passed, failed);

    cout << "Test: SST Get - Get The Newest Value" << endl;
    const pair<bool, int> &pair = controller.get(10);
    assert(true == pair.first);
    checkTestResult<int>(15, pair.second, passed, failed);

    cout << "Test: SST Get - Non-existing Key" << endl;
    const ::pair<bool, int> &pair2 = controller.get(100);
    assert(false == pair2.first);
    checkTestResult<int>(-1, pair2.second, passed, failed);

    cout << "Test: SST Scan" << endl;
    const vector<array<int, 2>> &scanResult = controller.scan(19, 67);
    expectedKvPairs =
        "(20,25) (23,123) (24,124) (25,125) (30,35) (40,45) (50,55) (60,65) ";
    checkTestResult<string>(expectedKvPairs, stringifyKvPairs(scanResult),
                            passed, failed);

    // Clean up test data
    controller.deleteFiles();

    // Summary of tests completed
    cout << "Tests completed: " << passed << "/" << (passed + failed)
         << " passed." << endl;
    array<int, 2> passFail = {passed, failed};
    return passFail;
}

int readIntFromPath(const string &expectedMetaDataPath) {
    ifstream inputFile(expectedMetaDataPath);
    int result;
    inputFile >> result;
    inputFile.close();
    return result;
}

array<int, 2> runBufferPoolTests() {
    cout << "\n" << endl;
    cout << "#################################" << endl;
    cout << "# Running Buffer Pool tests..." << endl;
    cout << "#################################" << endl;

    // Setup
    int passed = 0;
    int failed = 0;

    cout << "Test: Hash function consistency" << endl;
    std::string input = "Hello, World!";
    std::string input2 = "Hello, World!";
    int seed = 123;
    int expected = XXHash32::hash(input.c_str(), input.size(), seed);
    int actual = XXHash32::hash(input2.c_str(), input2.size(), seed);
    checkTestResult<int>(expected, actual, passed, failed);

    return {passed, failed};
}

array<int, 2> runBTreeTests() {
    cout << "\n" << endl;
    cout << "#################################" << endl;
    cout << "# Running B tree tests..." << endl;
    cout << "#################################" << endl;

    // setup
    int passed = 0;
    int failed = 0;

    // init data
    int memSize = (1 << 20) / 8 * 1;  // 1mb total
    int bufferCapacity =
        (1 << 20) / 4096 * 10;  // ~10mb total, ignoring metadata
    KVStore kvStore = KVStore(memSize, "testDB", bufferCapacity);

    int totalKVPairs = memSize;

    for (int i = 0; i < totalKVPairs; i++) {
        kvStore.put(i, i);
    }

    kvStore.createStaticBTree();
    int res = kvStore.bTreeGet(10);
    cout << "Test: B-tree get correct values" << endl;
    int expected = kvStore.bTreeGet(10);
    int actual = 10;
    checkTestResult<int>(expected, actual, passed, failed);
    
    expected = kvStore.bTreeGet(511);
    actual = 511;
    checkTestResult<int>(expected, actual, passed, failed);

    kvStore.deleteDb();

    return {passed, failed};
}

int main() {
    vector<array<int, 2>> passFails;

    // passFails.push_back(runAVLTreeTests());
    // passFails.push_back(runSSTControllerTests());
    // passFails.push_back(runBufferPoolTests());
    passFails.push_back(runBTreeTests());

    // calculate the total number of passed/failed tests
    int passed = 0;
    int failed = 0;
    for (auto passFail : passFails) {
        passed += passFail[0];
        failed += passFail[1];
    }

    cout << "\n" << endl;
    cout << "#################################" << endl;
    cout << "# Total Summary: " << passed << "/" << (passed + failed)
         << " passed." << endl;
    cout << "#################################" << endl;

    return 0;
}
