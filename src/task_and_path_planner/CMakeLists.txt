cmake_minimum_required(VERSION 3.16)
project(task_and_path_planner)

set(CMAKE_CXX_STANDARD 17)

find_package(spdlog REQUIRED)
find_package(Middleware-Node REQUIRED)
find_package(Middleware-Msgs REQUIRED)

include_directories(
    include
)

# PathCalculator library
add_library(PathCalculator
    src/PathCalculator.cpp
)

target_link_libraries(PathCalculator
    Middleware-Node::Middleware-Node
    Middleware-Msgs::TaskPlanner
    spdlog::spdlog
)

# Main executable
add_executable(path_calculator_node
    src/main.cpp
)

target_link_libraries(path_calculator_node
    PathCalculator
    Middleware-Node::Middleware-Node
    Middleware-Msgs::TaskPlanner
    spdlog::spdlog
)

# Test executable
add_executable(PathCalculatorTest
    test/PathCalculatorTest.cpp
)

target_link_libraries(PathCalculatorTest
    PathCalculator
    Middleware-Node::Middleware-Node
    Middleware-Msgs::TaskPlanner
    spdlog::spdlog
)

# Standalone test (doesn't require middleware infrastructure)
add_executable(StandaloneTest
    test/StandaloneTest.cpp
)

target_link_libraries(StandaloneTest
    spdlog::spdlog
)