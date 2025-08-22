cmake_minimum_required(VERSION 3.10)
project(huffman-encoder)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

add_executable(huffman-encoder main.cpp)
