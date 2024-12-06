#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include "Constants.h"
#include "LSMStore.h"

using namespace std;
using namespace std::chrono;

// Function to measure throughput and run the experiment
void run_experiment(int totalKVPairs, int intervals, int queries,
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

    // init data
    int memSize = (1 << 20) / 8 * 1;  // 1mb total
    int bufferCapacity =
        (1 << 20) / 4096 * 10;  // ~10mb total, ignoring metadata
    shared_ptr<LSMStore> lsmStore =
        make_shared<LSMStore>(memSize, "testDB", bufferCapacity);

    int prev_size = 0;
    for (int size = intervalSize; size <= totalKVPairs;
         size = size + intervalSize) {
        double dataSizeGB = (double)size * KVPAIR_SIZE / (1 << 30);
        cout << "-------------Running data size: " << std::fixed
             << std::setprecision(3) << dataSizeGB << "GB-------------" << endl;

        for (int i = prev_size; i < size; i++) {
            lsmStore->put(i, i);
        }
        prev_size = size;

        cout << "Querying pages" << endl;
        auto start_time = high_resolution_clock::now();

        // create a random number generator for the key selection
        random_device rd;
        mt19937 rng(rd());
        uniform_int_distribution<> dis(0, size - 1);

        // perform the queries
        for (int i = 0; i < queries; i++) {
            int random_key = dis(rng);  // generate a random key
            try {
                lsmStore->get(random_key);
            } catch (const runtime_error& e) {
                cerr << "key not found, " << "curr size: " << size
                     << ", key: " << random_key << endl;
            }
        }

        auto end_time = high_resolution_clock::now();
        auto duration_milliseconds =
            duration_cast<milliseconds>(end_time - start_time);
        double duration_seconds = duration_milliseconds.count() / 1000.0;

        // calculate throughput (KB per second)
        double throughput =
            static_cast<double>(queries * KVPAIR_SIZE / (1 << 10)) /
            duration_seconds;

        // output to csv
        output_file << dataSizeGB << "," << throughput << ","
                    << duration_seconds << "\n";
        cout << std::setprecision(4) << "Throughput: " << throughput
             << " KB/s | Time Taken: " << duration_seconds << " s\n";
    }

    lsmStore->deleteDb();
    output_file.close();
}

int main() {
    // test data sizes (number of key value pairs)
    int totalKVPairs = (1 << 30) / KVPAIR_SIZE;  // 1GB data size
    int intervals = 8;                           // interval size
    int queries = (1 << 10) / 8;                 // query 1KB of data

    run_experiment(totalKVPairs, intervals, queries, "query_throughput.csv");

    return 0;
}
