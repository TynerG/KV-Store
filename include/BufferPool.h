#ifndef BUFFERPOOL_H
#define BUFFERPOOL_H

#include <array>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct BufferFrame {
    BufferFrame* next;  // Next frame in the chaining (linked list)

    string pageId;                  // {sst num}-{page num}
    vector<array<int, 2>> kvPairs;  // Page data (4KB) or less
    bool isDirty;  // Flag to indicate if the page has been modified
    bool refBit;   // Bit used by clock to mark page access

    BufferFrame(string pageId, vector<array<int, 2>> kvPairs);
};

class BufferPool {
   private:
    int capacity;  // Number of buffer frames
    int numPages;  // Number of pages in the buffer pool
    vector<BufferFrame*>
        bufferFrames;             // List of buffer frames ptrs for chaining
    int clockHand;                // Position of the clock hand in bufferFrames

    // Hashes pageId into an index of bufferFrame
    int hashPageIdToIndex(string pageId);

    // Evicts a page using clock
    void evictPage();

   public:
    BufferPool(int capacity);

    // Destructor to free up the allocated memory for linked lists
    ~BufferPool();

    // Returns index of a page in buffer pool, or -1 if not found
    int findPage(int sstIdx, int pageNum);

    // Returns KV Pairs in a page, or empty vector if page not in buffer pool
    vector<array<int, 2>> getPage(string pageId);

    // Inserts a page into buffer pool
    void putPage(string pageId, vector<array<int, 2>> kvPairs);

    void updatePage(int sstIdx, int pageNum, vector<array<int, 2>> kvPairs);

    // Generates a pageId for the given sstIdx and pageNum
    string makePageId(int sstIdx, int pageNum);

    string makeLeveledPageId(int sstLevel, int sstIdx, int pageNum);

    // Returns the capacity of this buffer pool
    int getCapacity();
};

#endif