# Set the minimum version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(AVLTreeProject)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(include)

# Add the main executable
add_executable(main main.cpp 
        src/AVLTree.cpp
        src/KVStore.cpp
        src/SSTController.cpp
        src/BufferPool.cpp)

# Add the test executable
add_executable(tests tests/tests.cpp 
        src/AVLTree.cpp
        src/KVStore.cpp
        src/SSTController.cpp
        src/BufferPool.cpp)

# Add the queryThroughput executable
add_executable(queryThroughput tests/queryThroughput.cpp
        src/AVLTree.cpp
        src/KVStore.cpp
        src/SSTController.cpp
        src/BufferPool.cpp)
