cmake_minimum_required(VERSION 3.16)
project(Middleware-Util)

set(CMAKE_CXX_STANDARD 20)

find_package(Middleware-Node REQUIRED)
find_package(Middleware-Msgs REQUIRED)


add_executable(middleware-util src/main.cpp)
target_link_libraries(middleware-util Middleware-Node::Middleware-Node  Middleware-Msgs::Std)