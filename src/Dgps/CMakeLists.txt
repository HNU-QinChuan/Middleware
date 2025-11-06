cmake_minimum_required(VERSION 3.16)


project(dgps)

    set(CMAKE_CXX_STANDARD 17)

include_directories(src)

set(CONFIG_DIR ${CMAKE_SOURCE_DIR}/src/Dgps/yaml)
set(DEST_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DgpsConfig) 
file(MAKE_DIRECTORY ${DEST_DIR})

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

add_custom_command(
    TARGET dgps POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CONFIG_DIR} ${DEST_DIR}
)
