#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include "Constants.h"
#include "KVStore.h"

using namespace std;
using namespace std::chrono;

// Function to measure throughput of B-tree get operation
void run_btree_get_experiment(int totalKVPairs, int intervals, int queries,
                              const string& filename) {
    int intervalSize = static_cast<int>(totalKVPairs / intervals);

    // Open the CSV file to save results
    ofstream output_file(filename);
    if (!output_file.is_open()) {
        cerr << "Error opening file for writing results." << endl;
        return;
    }

    // Write the header of the CSV file
    output_file << "Data Size (GB),Throughput (KB/s),Time Taken (s)\n";

    // Initialize data for the experiment
    int memSize = (1 << 20) / 8 * 1;  // 1MB total
    int bufferCapacity =
        (1 << 20) / 4096 * 10;  // ~10MB total, ignoring metadata
    KVStore kvStore(memSize, "testDB", bufferCapacity);

    int prev_size = 0;
    for (int size = intervalSize; size <= totalKVPairs;
         size = size + intervalSize) {
        double dataSizeGB = (double)size * KVPAIR_SIZE / (1 << 30);
        cout << "-------------Running data size: " << std::fixed
             << std::setprecision(3) << dataSizeGB << "GB-------------" << endl;

        // Insert key-value pairs up to the current size
        for (int i = prev_size; i < size; i++) {
            kvStore.put(i, i);
        }
        prev_size = size;

        // Create the B-tree
        kvStore.createStaticBTree();

        cout << "Querying with B-tree" << endl;
        auto start_time = high_resolution_clock::now();

        // Create a random number generator for the key selection
        random_device rd;
        mt19937 rng(rd());
        uniform_int_distribution<> dis(0, size - 1);

        // Perform the queries
        for (int i = 0; i < queries; i++) {
            int random_key = dis(rng);  // Generate a random key
            try {
                int value = kvStore.bTreeGet(random_key);  // Use B-tree search
                // cout << "key: " << random_key << endl;
                // cout << "value: " << value << endl;
            } catch (const runtime_error& e) {
                cerr << "key not found, " << "curr size: " << size
                     << ", key: " << random_key << endl;
            }
        }

        auto end_time = high_resolution_clock::now();
        auto duration_milliseconds =
            duration_cast<milliseconds>(end_time - start_time);
        double duration_seconds = duration_milliseconds.count() / 1000.0;

        // Calculate throughput (KB per second)
        double throughput =
            static_cast<double>(queries * KVPAIR_SIZE / (1 << 10)) /
            duration_seconds;

        // Output results to CSV
        output_file << dataSizeGB << "," << throughput << ","
                    << duration_seconds << "\n";
        cout << std::setprecision(4) << "Throughput: " << throughput
             << " KB/s | Time Taken: " << duration_seconds << " s\n";
    }

    kvStore.deleteDb();  // Clean up after the experiment
    output_file.close();
}

int main() {
    // Test data sizes (number of key-value pairs)
    int totalKVPairs = (1 << 30) / KVPAIR_SIZE;  // 1GB data size
    int intervals = 8;            // Interval size for different data sizes
    int queries = (1 << 10) / 8;  // Query 1KB of data (can adjust as needed)

    run_btree_get_experiment(totalKVPairs, intervals, queries,
                             "btree_get_throughput.csv");

    return 0;
}
