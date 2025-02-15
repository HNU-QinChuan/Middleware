cmake_minimum_required(VERSION 3.16)
project(Middleware-Node)

set(CMAKE_CXX_STANDARD 20)


include_directories(include)

add_library(${PROJECT_NAME} SHARED
        src/Node.cpp
        include/Node.hpp)

add_executable(${PROJECT_NAME}_test src/main.cpp)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})