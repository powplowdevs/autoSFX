cmake_minimum_required(VERSION 3.10)

# Project Name
project(SFXMaker)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Add the executable
add_executable(SFXMaker src/main.cpp src/SFXMaker.cpp)