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
        include/AVLTree.h
        src/AVLTree.cpp
#        include/KVStore.h
#        src/KVStore.cpp
#        include/SSTController.h
#        src/SSTController.cpp
        include/BufferPool.h
        src/BufferPool.cpp
        include/LSMController.h
        src/LSMController.cpp
        include/LSMStore.h
        src/LSMStore.cpp)

## Add the test executable
#add_executable(tests tests/tests.cpp
#        include/AVLTree.h
#        src/AVLTree.cpp
##        include/KVStore.h
##        src/KVStore.cpp
##        include/SSTController.h
##        src/SSTController.cpp
#        include/BufferPool.h
#        src/BufferPool.cpp
#        include/LSMController.h
#        src/LSMController.cpp
#        include/LSMStore.h
#        src/LSMStore.cpp)

## Add the queryThroughput executable
#add_executable(queryThroughput tests/queryThroughput.cpp
#        include/AVLTree.h
#        src/AVLTree.cpp
#        include/KVStore.h
#        src/KVStore.cpp
#        include/SSTController.h
#        src/SSTController.cpp
#        include/BufferPool.h
#        src/BufferPool.cpp
#        include/LSMController.h
#        src/LSMController.cpp
#        include/LSMStore.h
#        src/LSMStore.cpp)
