# Introduction

In this project, we present the implementation of a key-value (KV) store as part of the course project for CSC443. The database uses a combination of in-memory structures and disk storage to manage large amounts of data in gigabytes. This implementation uses a Memtable, Sorted String Tables (SSTs), a buffer pool, and a LSM Tree with a Bloom filter, and advanced query techniques to optimize for both storage and retrieval performance. Feel free to check out our [Report](report/report.pdf) for a detailed description of the project.

## File Structure
This project is developed for the CSC443 course. It includes a main application along with test files that can be built using instructions below. The structure of this directory is as follows:
- `report`: contains our report in report.pdf, and data from experiments.
- `src`: contains source files.
- `include`: contains header files.
- `tests`: contains test files.

## Prequisites
- Ensure you have CMake installed
- The project uses C++17

## Report
Please see the [Report](report/report.pdf) for a detailed description of the project.

## Compile

To compile the project, follow these steps:

Create a Build Directory:
```bash
mkdir build
cd build
```

Run CMake to Configure the Project:
   ```bash
   cmake ..
   ```

Build/rebuild the Project:
```bash
make
```

## Execute

After compiling the project, you can run the executables in the `build` folder:

To run the main program:
```bash
./main
```

To run the tests:
```bash
./tests
```

## Release
Can dramatic improve the performance simply by enabling compiler optimization:
```bash
mkdir build-release
cd build-release
cmake ../ -DCMAKE_BUILD_TYPE=Release
make
```

