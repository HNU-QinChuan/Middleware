cmake_minimum_required(VERSION 3.16)
project(local_planner)

set(CMAKE_CXX_STANDARD 17)
set(CONFIG_DIR ${CMAKE_SOURCE_DIR}/src/LocalPlanner/yaml)
set(DEST_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/LocalPlannerConfig) 
file(MAKE_DIRECTORY ${DEST_DIR})

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
aux_source_directory(src/car car)

add_executable(car_planner
        src/car/CarDwaNode.cpp
        ${base}
        ${car}
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

add_executable(LineTest
        test/LineTest.cpp)
        
add_executable(MotionStateTest
        test/MotionStateTest.cpp)   

add_executable(GetPath
        test/GetPath.cpp)

        
add_custom_command(
    TARGET car_planner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CONFIG_DIR} ${DEST_DIR}
)