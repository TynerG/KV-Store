#ifndef CONSTANTS_H
#define CONSTANTS_H

constexpr int PAGE_SIZE = 4096;  // Size of one page in bytes
constexpr int KVPAIR_SIZE = 8;   // Size of one key-value pair
constexpr int B =
    PAGE_SIZE /
    KVPAIR_SIZE;  // Number of key-value pairs that can fit in one page
constexpr int T = 1;  // T for LSM-tree

#endif  // CONSTANTS_H