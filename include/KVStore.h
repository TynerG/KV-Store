//
// A class for the KV Store
//

#ifndef AVLTREEPROJECT_KVSTORE_H
#define AVLTREEPROJECT_KVSTORE_H

#include "./AVLTree.h"
#include "./SSTController.h"

class KVStore {
    AVLTree memtable;
    SSTController sstController;
};


#endif //AVLTREEPROJECT_KVSTORE_H
