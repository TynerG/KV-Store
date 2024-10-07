#include <iostream>
#include <memory>

#include "include/AVLTree.h"
#include "SSTController.h"
#include "KVStore.h"

using namespace std;

// TODO: Remove these hard coded value with arguments from the user.
int MEMTABLE_CAPACITY = 10;
string DATABASE_NAME = "MyDatabase";

int main(int argc, char *argv[]) {
    cout << "hello world" << endl;
    // reassign memtable to a new AVLTree to automatically free the old one
    // (implemented with smart pointer and destructor)
    shared_ptr<KVStore> db = make_shared<KVStore>(MEMTABLE_CAPACITY, DATABASE_NAME);

    return 0;
}
