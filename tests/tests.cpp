#include <iostream>

#include "../include/AVLTree.h"

using namespace std;

// Function template to compare results of generic types
template <typename T>
void checkTestResult(const T& expected, const T& result, int& passed,
                     int& failed) {
    if (result == expected) {
        passed++;
        cout << "--[Passed]" << endl;
    } else {
        failed++;
        cout << "--[Failed]" << endl;
        cout << "Expected '" << expected << "', got '" << result << "'" << endl;
    }
}

string stringify_kv_pairs(const vector <array<int, 2>>& the_pair) {
    string result;
    for (auto pair: the_pair){
        result += to_string(pair[0]) + " ";
    }

    return result;
}

void runAVLTreeTests() {
    cout << "Running AVL tree tests..." << endl;

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
    string expected = "25 30 40 50 60 ";
    checkTestResult(expected, stringify_kv_pairs(tree.scan(23, 69)), passed, failed);

    cout << "Test: Scan Inclusive" << endl;
    expected = "20 25 30 40 50 60 70 ";
    checkTestResult(expected, stringify_kv_pairs(tree.scan(20, 70)), passed, failed);

    // Summary of tests completed
    cout << "Tests completed: " << passed << "/" << (passed + failed)
         << " passed." << endl;
}

int main() {
    runAVLTreeTests();
    return 0;
}
