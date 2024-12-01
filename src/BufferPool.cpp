#include "BufferPool.h"

#include "Constants.h"
#include "xxHash32.h"

int SEED = 123;

BufferFrame::BufferFrame(string pageId, vector<array<int, 2>> kvPairs)
    : pageId(pageId),
      kvPairs(kvPairs),
      isDirty(false),
      next(nullptr),
      refBit(true) {};

BufferPool::BufferPool(int capacity)
    : capacity(capacity), numPages(0), clockHand(0) {
    bufferFrames.resize(capacity, nullptr);  // Init with nullptrs
}

BufferPool::~BufferPool() {
    for (auto& head : bufferFrames) {
        while (head != nullptr) {
            BufferFrame* temp = head;
            head = head->next;
            delete temp;  // free memory for each linked list node
        }
    }
}

string BufferPool::makePageId(int sstIdx, int pageNum) {
    return to_string(sstIdx) + "-" + to_string(pageNum);
}

int BufferPool::hashPageIdToIndex(string pageId) {
    return XXHash32::hash(pageId.c_str(), pageId.size(), SEED) % capacity;
}

int BufferPool::findPage(int sstIdx, int pageNum) {
    string pageId = makePageId(sstIdx, pageNum);
    int idx = hashPageIdToIndex(pageId);

    BufferFrame* curr = bufferFrames[idx];
    while (curr != nullptr) {
        if (curr->pageId == pageId) return idx;
        curr = curr->next;
    }
    return -1;  // page not found
};

vector<array<int, 2>> BufferPool::getPage(int sstIdx, int pageNum) {
    // cout << sstIdx << "," << pageNum << endl;
    string pageId = makePageId(sstIdx, pageNum);
    int idx = hashPageIdToIndex(pageId);

    BufferFrame* curr = bufferFrames[idx];
    while (curr != nullptr) {
        if (curr->pageId == pageId) {
            curr->refBit = true;  // mark as recently used
            return curr->kvPairs;
        }
        curr = curr->next;
    }
    // cout << "page not found" << endl;
    return {};
}

void BufferPool::putPage(int sstIdx, int pageNum,
                         vector<array<int, 2>> kvPairs) {
    if (numPages == capacity) {
        evictPage();
    }

    // insert page
    string pageId = makePageId(sstIdx, pageNum);
    int idx = hashPageIdToIndex(pageId);
    BufferFrame* curr = bufferFrames[idx];
    BufferFrame* prev = nullptr;

    while (curr != nullptr) {
        if (curr->pageId == pageId) return;  // page already exists
        prev = curr;
        curr = curr->next;
    }

    BufferFrame* newFrame = new BufferFrame(pageId, kvPairs);
    if (prev == nullptr) {
        // if the linked list is empty, insert as the first element
        bufferFrames[idx] = newFrame;
    } else {
        prev->next = newFrame;
    }
    numPages++;
}

void BufferPool::updatePage(int sstIdx, int pageNum,
                            vector<array<int, 2>> kvPairs) {
    string pageId = makePageId(sstIdx, pageNum);
    int idx = hashPageIdToIndex(pageId);

    BufferFrame* curr = bufferFrames[idx];
    while (curr != nullptr) {
        if (curr->pageId == pageId) {
            curr->kvPairs = kvPairs;
            curr->isDirty = true;
            return;
        }
        curr = curr->next;
    }
}

void BufferPool::evictPage() {
    // cout << "Begin eviction" << endl;
    while (true) {
        BufferFrame* frame = bufferFrames[clockHand];

        if (frame == nullptr) {
            clockHand = (clockHand + 1) % capacity;
            continue;
        }

        // if the reference bit is 0, evict the page
        if (!frame->refBit) {
            // cout << "Evicted page" << endl;

            // TODO: if the page is dirty, write it back to storage
            if (frame->isDirty) {
                cout << "Writing dirty page not implemented!" << endl;
            }

            // remove the page from buffer
            if (frame->next != nullptr) {
                bufferFrames[clockHand] = frame->next;
            } else {
                bufferFrames[clockHand] = nullptr;
            }
            delete frame;

            numPages--;
            break;
        } else {
            // mark the page as not recently used
            frame->refBit = false;
            clockHand = (clockHand + 1) % capacity;
        }
    }
}