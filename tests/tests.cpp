#include <iostream>
#include <sys/stat.h>

#include "../include/AVLTree.h"
#include "../include/SSTController.h"

int readIntFromPath(const string &expectedMetaDataPath);

using namespace std;

// Function template to compare results of generic types
template<typename T>
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
    for (auto pair: the_pair) {
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
    checkTestResult(expected, stringifyKvPairs(tree.scan(23, 69)), passed, failed);

    cout << "Test: Scan Inclusive" << endl;
    expected = "(20,20) (25,123) (30,30) (40,40) (50,50) (60,60) (70,60) ";
    checkTestResult(expected, stringifyKvPairs(tree.scan(20, 70)), passed, failed);

    // Summary of tests completed
    cout << "Tests completed: " << passed << "/" << (passed + failed)
         << " passed." << endl;

    array<int, 2> passFail = {passed, failed};
    return passFail;
}

array<int, 2> runSSTControllerTests(bool isCleanUpTestData) {
    cout << "\n" << endl;
    cout << "#################################" << endl;
    cout << "# Running SST Controller tests..." << endl;
    cout << "#################################" << endl;

    // Setup
    int passed = 0;
    int failed = 0;

    // build KV-Pairs
    AVLTree tree(8);  // Assuming 8 is the initial capacity or some parameter
    tree.insert(10, 10);
    tree.insert(20, 20);
    tree.insert(23, 123);
    tree.insert(24, 124);
    tree.insert(25, 125);
    tree.insert(30, 30);
    tree.insert(40, 40);
    tree.insert(50, 50);
    const vector<array<int, 2>> &kvPairs = tree.scan();

    // init the controller
    const string dbName = "MyDatabase";
    SSTController controller(dbName);

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
    string expectedPathToSST = "./MyDatabase/sst-1";
    controller.save(kvPairs);
    const vector<array<int, 2>> &savedKvPairs = controller.read(1);
    string expectedKvPairs = "(10,10) (20,20) (23,123) (24,124) (25,125) (30,30) (40,40) (50,50) ";
    checkTestResult<string>(expectedKvPairs, stringifyKvPairs(savedKvPairs), passed, failed);

    cout << "Test: Maintains Metadata" << endl;
    int expectedMetadata = 1;
    int actualMetadata = readIntFromPath(expectedMetaDataPath);
    checkTestResult<int>(expectedMetadata, actualMetadata, passed, failed);

    cout << "Test: Open Existing Database" << endl;
    expectedMetadata = 1;
    SSTController controllerForExistingDb(dbName);
    actualMetadata = controllerForExistingDb.getMetadata();
    checkTestResult<int>(expectedMetadata, actualMetadata, passed, failed);

    // Clean up test data
    if (isCleanUpTestData) {
        // TODO: implement this after confirming we can use C++ 17 (it makes this much easier)
        //  for now, manually remove everything under and including ./MyDatabase
    }

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

int main() {
    vector<array<int, 2>> passFails;

    passFails.push_back(runAVLTreeTests());
    passFails.push_back(runSSTControllerTests(false));

    // calculate the total number of passed/failed tests
    int passed = 0;
    int failed = 0;
    for (auto passFail: passFails) {
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
