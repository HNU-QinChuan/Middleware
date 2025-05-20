cmake_minimum_required(VERSION 3.16)
project(Middleware-Test)

set(CMAKE_CXX_STANDARD 17)

# file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto")
# execute_process(COMMAND protoc ${PROTO_FILES} --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/src -I=${CMAKE_CURRENT_SOURCE_DIR})


find_package(Middleware-Node REQUIRED)
find_package(Middleware-Msgs REQUIRED)
# find_package(Protobuf REQUIRED)
# find_package(spdlog REQUIRED)


# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(pub src/pub.cpp)
target_link_libraries(pub Middleware-Node::Middleware-Node  Middleware-Msgs::Std)

add_executable(sub src/sub.cpp)
target_link_libraries(sub Middleware-Node::Middleware-Node  Middleware-Msgs::Std)

add_executable(topicecho src/topicecho.cpp)
target_link_libraries(topicecho Middleware-Node::Middleware-Node  Middleware-Msgs::Std jsoncpp)
target_link_options(topicecho PRIVATE "-Wl,--no-as-needed")

add_executable(topicpub src/topicpub.cpp)
target_link_libraries(topicpub Middleware-Node::Middleware-Node  Middleware-Msgs::Std jsoncpp)
target_link_options(topicpub PRIVATE "-Wl,--no-as-needed")