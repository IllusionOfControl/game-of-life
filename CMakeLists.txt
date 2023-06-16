cmake_minimum_required(VERSION 3.24)
project(game-of-life)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_BUILD_TYPE Debug)

set(SOURCE_FILES ${SOURCE_FILES} src/main.cpp)
add_executable(game-of-life WIN32 ${SOURCE_FILES})

target_link_libraries(game-of-life)