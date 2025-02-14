cmake_minimum_required(VERSION 3.16)   
project(Middleware)          

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src/Master)
add_subdirectory(src/Node)

add_executable(test src/main.cpp)