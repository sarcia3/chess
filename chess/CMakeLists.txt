cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(chess VERSION 1.0)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_executable(chess main.cpp ${SRC_FILES})
target_include_directories(chess PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

