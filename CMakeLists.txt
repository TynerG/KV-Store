# Set the minimum version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(AVLTreeProject)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(include)

# Add the main executable
add_executable(main main.cpp src/AVLTree.cpp
        include/KVStore.h
        src/KVStore.cpp
        src/SSTController.cpp
        include/SSTController.h
        src/BufferPool.cpp
        include/BufferPool.h)

# Add the test executable
add_executable(tests tests/tests.cpp src/AVLTree.cpp
        include/KVStore.h
        src/KVStore.cpp
        src/SSTController.cpp
        include/SSTController.h
        src/BufferPool.cpp
        include/BufferPool.h)

# Optionally link with testing frameworks (e.g., Google Test) if required
# target_link_libraries(run_tests gtest gtest_main)
