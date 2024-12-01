#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "KVStore.h"

using namespace std;

void show_help() {
    cout << "Available commands:" << endl;
    cout << "  put <key> <value>          : Inserts a key-value pair into the "
            "KV store."
         << endl;
    cout << "  get <key>                  : Retrieves the value associated "
            "with a given key."
         << endl;
    cout << "  scan <low> <high>          : Scans and retrieves all KV-pairs "
            "in the range [low, high]."
         << endl;
    cout << "  open <dbName> <memtableSize> <bufferCapacity> : Opens the KV "
            "store with the specified parameters."
         << endl;
    cout << "  close                      : Closes the KV store and saves the "
            "memtable to disk."
         << endl;
    cout
        << "  delete                     : Deletes the current opened KV store."
        << endl;
    cout << "  exit                       : Exits the program." << endl;
    cout << "  help                       : Displays this help message."
         << endl;
}

int main() {
    shared_ptr<KVStore> kvStore = nullptr;
    string command;
    bool isDbOpen = false;

    cout << "Welcome to KV Store DB. Type 'help' for available commands."
         << endl;
    while (true) {
        getline(cin, command);

        stringstream stringstream(command);
        string cmd;
        stringstream >> cmd;
        string dbName;

        if (cmd == "help") {
            show_help();
            continue;
        } else if (cmd == "exit") {
            break;
        }

        if (!isDbOpen) {
            if (cmd == "open") {
                int memtableSize, bufferCapacity;  // 10, 4
                stringstream >> dbName >> memtableSize >> bufferCapacity;

                if (dbName.empty() || memtableSize <= 0 ||
                    bufferCapacity <= 0) {
                    cout << "Invalid parameters for open command. Correct "
                            "usage: open <dbName> <memtableSize> "
                            "<bufferCapacity>"
                         << endl;
                } else {
                    kvStore = make_shared<KVStore>(memtableSize, dbName,
                                                   bufferCapacity);
                    cout << "Opened KV Store with DB: " << dbName << endl;
                    isDbOpen = true;
                }
            } else {
                cout << "Please first open DB connection before executing "
                        "other commands."
                     << endl;
            }
        } else {
            if (cmd == "put") {
                int key, value;
                if (stringstream >> key >> value) {
                    if (kvStore && kvStore->put(key, value)) {
                        cout << "Inserted (" << key << ", " << value
                             << ") into the KV store." << endl;
                    } else {
                        cout << "Failed to insert KV pair." << endl;
                    }
                } else {
                    cout << "Invalid parameters for put command. Correct "
                            "usage: put <key> <value>"
                         << endl;
                }
            } else if (cmd == "get") {
                int key;
                if (stringstream >> key) {
                    if (kvStore) {
                        try {
                            int value = kvStore->get(key);
                            cout << "Value for key " << key << ": " << value
                                 << endl;
                        } catch (const std::runtime_error& e) {
                            cout << "Key " << key << " doesn't exist." << endl;
                        }
                    }
                } else {
                    cout << "Invalid parameters for get command. Correct "
                            "usage: get <key>"
                         << endl;
                }
            } else if (cmd == "scan") {
                int low, high;
                if (stringstream >> low >> high) {
                    if (kvStore) {
                        vector<array<int, 2>> result = kvStore->scan(low, high);
                        cout << "Scan results from " << low << " to " << high
                             << ": " << endl;
                        for (auto& pair : result) {
                            cout << pair[0] << ": " << pair[1] << endl;
                        }
                    }
                } else {
                    cout << "Invalid parameters for scan command. Correct "
                            "usage: scan <low> <high>"
                         << endl;
                }
            } else if (cmd == "close") {
                if (kvStore && kvStore->close()) {
                    cout << "KV Store closed successfully." << endl;
                    isDbOpen = false;
                } else {
                    cout << "Failed to close KV Store." << endl;
                }
            } else if (cmd == "delete") {
                if (kvStore) {
                    kvStore->deleteDb();
                    isDbOpen = false;
                    cout << "Database " << dbName << " deleted." << endl;
                } else {
                    cout << "No open database to delete." << endl;
                }
            } else {
                cout << "Invalid command." << endl;
            }
        }
    }

    return 0;
}
