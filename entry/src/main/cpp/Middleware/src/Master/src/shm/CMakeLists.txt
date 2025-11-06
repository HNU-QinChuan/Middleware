cmake_minimum_required(VERSION 3.16)
project(middleware_node_shm)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=1)

find_package(spdlog REQUIRED)

# 设置 include 目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# 收集源文件
aux_source_directory(. shm_sources)
aux_source_directory(controller shm_controller_sources)
aux_source_directory(../interface interface)
aux_source_directory(../interface/controller interface_controller)
aux_source_directory(../tcp tcp)

add_library(${PROJECT_NAME} SHARED
    ${shm_sources}
    ${shm_controller_sources}
    ${interface}
    ${interface_controller}
    ${tcp}
    ../MiddlewareManager.cpp
    ../ProcessManager.cpp
    ../InterfaceManager.cpp
)

target_link_libraries(${PROJECT_NAME}
    spdlog::spdlog
    jsoncpp.a
    rt
    libhilog_ndk.z.so
)

add_compile_definitions(BOOST_ASIO_HAS_STD_INVOKE_RESULT _LIBCPP_ENABLE_CXX17_REMOVED_FEATURES)

# 设置库输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/lib
)
