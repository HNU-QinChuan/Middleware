cmake_minimum_required(VERSION 3.16)
project(Middleware-Node)

set(CMAKE_CXX_STANDARD 20)

file(GLOB PROTO_FILES "${CMAKE_SOURCE_DIR}/src/Node/proto/*.proto")
execute_process(COMMAND protoc ${PROTO_FILES} --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/test -I=${CMAKE_CURRENT_SOURCE_DIR})

find_package(spdlog REQUIRED)
find_package(Protobuf REQUIRED)

include_directories(include)

add_library(${PROJECT_NAME} SHARED
        src/Node.cpp
        include/Node.hpp
        include/Publish.decl.hpp
        include/Publish.hpp
        include/PublishInterface.hpp
        include/SubscribeInterface.hpp
        include/Subscribe.decl.hpp
        include/Subscribe.hpp
)
target_link_libraries(${PROJECT_NAME} spdlog::spdlog protobuf::libprotobuf rt)

add_executable(pub test/pub.cpp test/proto/example.pb.cc)
target_include_directories(pub PRIVATE test)
target_link_libraries(pub ${PROJECT_NAME})

add_executable(sub test/sub.cpp test/proto/example.pb.cc)
target_include_directories(sub PRIVATE test)
target_link_libraries(sub ${PROJECT_NAME})