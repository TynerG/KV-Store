#include <iostream>
#include <memory>

#include "include/AVLTree.h"
using namespace std;

int MEMTABLE_CAPACITY = 10;

int main(int argc, char* argv[]) {
    cout << "hello world" << endl;
    // reassign memtable to a new AVLTree to automatically free the old one
    // (implemented with smart pointer and destructor)
    shared_ptr<AVLTree> memtable = make_shared<AVLTree>(MEMTABLE_CAPACITY);

    return 0;
}
