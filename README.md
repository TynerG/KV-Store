# CSC443-Fall-2024-Project

This project is developed for the CSC443 course. It implements an AVL tree data structure and includes a main application along with test files.

## Prequisites
- Ensure you have CMake installed
- The project uses C++11

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

