cmake_minimum_required(VERSION 3.10)
project(PgnChessTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler to GCC 12
set(CMAKE_C_COMPILER "/usr/local/bin/gcc-12")
set(CMAKE_CXX_COMPILER "/usr/local/bin/g++-12")

# Add Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
FetchContent_MakeAvailable(googletest)

# Include directories
include_directories(${gtest_SOURCE_DIR}/include)
include_directories(${gtest_SOURCE_DIR})
include_directories(include)

# Source files for tests
file(GLOB TEST_SOURCES "tests/*.cpp")

# Add a test executable
add_executable(pgnchesstests ${TEST_SOURCES})

# Link against Google Test and your project library or source files
target_link_libraries(pgnchesstests gtest gtest_main)
target_link_libraries(pgnchesstests pgnchess) # Link against your project's main executable or library

# Add the tests to CTest
include(GoogleTest)
gtest_discover_tests(pgnchesstests)
