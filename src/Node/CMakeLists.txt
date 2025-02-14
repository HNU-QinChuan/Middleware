cmake_minimum_required(VERSION 3.16)
project(Middleware-Node)

set(CMAKE_CXX_STANDARD 20)


include_directories(include)

add_executable(${PROJECT_NAME} src/main.cpp)