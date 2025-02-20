cmake_minimum_required(VERSION 3.10)

# Project name and language
project(AutoSFXProject LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable target
add_executable(generated_sfx src/main.cpp)

# Include the directory where your header file is located
target_include_directories(generated_sfx PRIVATE ${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include)

# Only set the static linking flags if using the GNU compiler
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif()

# Link against Windows Shell32 library
target_link_libraries(generated_sfx PRIVATE Shell32)
