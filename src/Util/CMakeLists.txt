cmake_minimum_required(VERSION 3.16)
project(Middleware-Util)

set(CMAKE_CXX_STANDARD 17)

find_package(Middleware-Node REQUIRED)
find_package(Middleware-Msgs REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(middleware-util src/main.cpp)

target_link_libraries(middleware-util 
Middleware-Node::Middleware-Node 
Middleware-Msgs::Std 
pthread jsoncpp protobuf)

target_link_options(middleware-util PRIVATE "-Wl,--no-as-needed")