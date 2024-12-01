#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include "KVStore.h"

using namespace std;
using namespace std::chrono;

// Function to measure throughput and run the experiment
void run_experiment(const vector<int>& data_sizes, int queries,
                    const string& filename) {
    // Open the CSV file to save results
    ofstream output_file(filename);
    if (!output_file.is_open()) {
        cerr << "Error opening file for writing results." << endl;
        return;
    }

    // Write the header of the CSV file
    output_file << "Data Size,Throughput (queries/sec),Time Taken (seconds)\n";

    // Iterate over each data size
    for (int size : data_sizes) {
        cout << "Running data size: " << size << endl;

        // init data
        int memSize = 512;
        int bufferCapacity = 100;
        shared_ptr<KVStore> kvStore =
            make_shared<KVStore>(memSize, "testDB", bufferCapacity);

        for (int i = 0; i < size; i++) {
            kvStore->put(i, i);
        }

        cout << "Querying pages" << endl;
        auto start_time = high_resolution_clock::now();

        // Create a random number generator for the key selection
        random_device rd;
        mt19937 rng(rd());
        uniform_int_distribution<> dis(0, size - 1);

        // Perform the queries
        for (int i = 0; i < queries; i++) {
            int random_key = dis(rng);  // generate a random key
            try {
                // kvStore->get(900);  // test if buffer works
                kvStore->get(random_key);
            } catch (const runtime_error& e) {
                cerr << "key not found" << endl;
            }
        }

        auto end_time = high_resolution_clock::now();
        auto duration_milliseconds =
            duration_cast<milliseconds>(end_time - start_time);
        double duration_seconds = duration_milliseconds.count() / 1000.0;

        // calculate throughput (queries per second)
        double throughput = static_cast<double>(queries) / duration_seconds;

        // output to csv
        output_file << size << "," << throughput << "," << duration_seconds
                    << "\n";
        cout << "Data size: " << size << " | Throughput: " << throughput
             << " queries/sec | Time Taken: " << duration_seconds
             << " seconds\n";

        kvStore->deleteDb();
    }

    output_file.close();
}

int main() {
    // test data sizes (number of key value pairs)
    vector<int> data_sizes = {10000, 30000, 50000, 70000, 90000, 110000, 130000, 150000, 170000, 190000};
    int queries = 10000;

    run_experiment(data_sizes, queries, "query_throughput.csv");

    return 0;
}
