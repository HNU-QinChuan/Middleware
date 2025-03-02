cmake_minimum_required(VERSION 3.10)

project(dgps)

    set(CMAKE_CXX_STANDARD 20)

include_directories(include)

find_package(Middleware-Node REQUIRED)
find_package(Middleware-Msgs REQUIRED)

add_library(dgps
        src/main.cpp
)

target_link_libraries(dgps Middleware-Node::Middleware-Node  Middleware-Msgs::Std)
