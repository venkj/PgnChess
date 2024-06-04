cmake_minimum_required(VERSION 3.10)
project(PgnChess)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler to GCC 12
set(CMAKE_C_COMPILER "/usr/local/bin/gcc-12")
set(CMAKE_CXX_COMPILER "/usr/local/bin/g++-12")

# Include directories
include_directories(include)

# Source files
file(GLOB SOURCES "src/*.cpp")

# Add main.cpp outside the src directory
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

# Create build directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build)

# Add an executable target
add_executable(pgnchess ${SOURCES})

add_custom_target(clean-build
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/build
    COMMENT "Cleaning build directory"
)
