cmake_minimum_required(VERSION 3.16)
project(Middleware-Msgs)

set(CMAKE_CXX_STANDARD 20)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/proto/Std/*.proto")
execute_process(COMMAND protoc ${PROTO_FILES} --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/src -I=${CMAKE_CURRENT_SOURCE_DIR}/proto)

# find_package(Protobuf REQUIRED)

aux_source_directory(src/Std Std)
add_library(Std SHARED ${Std})
target_include_directories(Std PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
# target_link_libraries(Std PUBLIC protobuf::libprotobuf)
add_library(${PROJECT_NAME}::Std ALIAS Std)


export(TARGETS Std FILE ${CMAKE_BINARY_DIR}/StdTargets.cmake NAMESPACE ${PROJECT_NAME}::)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    @ONLY)