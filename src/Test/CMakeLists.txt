cmake_minimum_required(VERSION 3.16)
project(Test)

set(CMAKE_CXX_STANDARD 20)

file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto")
execute_process(COMMAND protoc ${PROTO_FILES} --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/src -I=${CMAKE_CURRENT_SOURCE_DIR})


find_package(Middleware-Node REQUIRED)
find_package(Protobuf REQUIRED)
find_package(spdlog REQUIRED)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(pub src/pub.cpp src/proto/example.pb.cc)
target_link_libraries(pub Middleware-Node::Middleware-Node spdlog::spdlog protobuf::libprotobuf)

add_executable(sub src/sub.cpp src/proto/example.pb.cc)
target_link_libraries(sub Middleware-Node::Middleware-Node spdlog::spdlog protobuf::libprotobuf)
