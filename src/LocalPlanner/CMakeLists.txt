cmake_minimum_required(VERSION 3.16)
project(local_planner)

set(CMAKE_CXX_STANDARD 20)

find_package(spdlog REQUIRED)
find_package(Middleware-Node REQUIRED)
find_package(Middleware-Msgs REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Eigen3 REQUIRED)

include_directories(
        include
        ${EIGEN3_INCLUDE_DIR}
)

aux_source_directory(src/base base)

add_executable(car_planner
        src/test.cpp
        ${base}
#        src/BaseDwaPlanner.cpp
#        src/BaseScore.cpp
#        src/BaseGeneratePath.cpp
#        src/CarDwaPlanner.cpp
#        src/CarDwaNode.cpp
)

target_link_libraries(car_planner
        Middleware-Node::Middleware-Node
        Middleware-Msgs::Std
        Middleware-Msgs::Dgps
        Middleware-Msgs::Geometry
        Middleware-Msgs::BuiltinInterfaces
        Middleware-Msgs::Nav
        Middleware-Msgs::TaskPlanner
        yaml-cpp
)