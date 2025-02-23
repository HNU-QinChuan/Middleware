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
        include/PublishInterface.hpp
        include/SubscribeInterface.hpp
        include/Subscribe.decl.hpp
        include/Subscribe.hpp
)
target_link_libraries(${PROJECT_NAME} spdlog::spdlog protobuf::libprotobuf)

add_executable(pub test/pub.cpp test/proto/example.pb.cc)
target_include_directories(pub PRIVATE test)
target_link_libraries(pub ${PROJECT_NAME})

add_executable(sub test/sub.cpp test/proto/example.pb.cc)
target_include_directories(sub PRIVATE test)
target_link_libraries(sub ${PROJECT_NAME})