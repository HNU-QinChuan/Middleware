cmake_minimum_required(VERSION 3.16)


project(dgps)

    set(CMAKE_CXX_STANDARD 20)

include_directories(src)

find_package(Middleware-Node REQUIRED)
find_package(Middleware-Msgs REQUIRED)
find_package(yaml-cpp REQUIRED)

add_library(dgps
        src/dgps_hmw_node.cpp
        src/serial.cpp
        src/ntripclient.cpp
        src/serial.h
        src/ntrip.h
)
add_executable(dgps-node src/main.cpp)
add_executable(dgps-test test/test.cpp)

target_link_libraries(dgps
        Middleware-Node::Middleware-Node
        Middleware-Msgs::Std
        Middleware-Msgs::Dgps
        yaml-cpp)
target_link_libraries(dgps-node dgps)
target_link_libraries(dgps-test dgps)
