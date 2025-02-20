cmake_minimum_required(VERSION 3.16)
project(Middleware-Node)

set(CMAKE_CXX_STANDARD 20)


find_package(spdlog REQUIRED)
find_package(Protobuf REQUIRED)

include_directories(include)

add_library(${PROJECT_NAME} SHARED
        src/Node.cpp
        include/Node.hpp
        include/Publish.decl.hpp
        include/Publish.hpp
)
target_link_libraries(${PROJECT_NAME} spdlog::spdlog protobuf::libprotobuf)

add_executable(${PROJECT_NAME}_test test/main.cpp test/proto/example.pb.cc)
target_include_directories(${PROJECT_NAME}_test PRIVATE test)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})