cmake_minimum_required(VERSION 3.16)   
project(Middleware)          

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

add_compile_definitions(BOOST_ASIO_HAS_STD_INVOKE_RESULT _LIBCPP_ENABLE_CXX17_REMOVED_FEATURES)



add_subdirectory(src/Msgs)
add_subdirectory(src/Master)
add_subdirectory(src/Node)
add_subdirectory(src/Test)
add_subdirectory(src/Dgps)
add_subdirectory(src/Util)
add_subdirectory(src/LocalPlanner)

