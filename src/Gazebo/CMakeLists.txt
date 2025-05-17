cmake_minimum_required(VERSION 3.16)

project(Gazebo)

set(CMAKE_CXX_STANDARD 17)

find_package(spdlog REQUIRED)
find_package(Middleware-Node REQUIRED)
find_package(Middleware-Msgs REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(gazebo REQUIRED)

include_directories(
  ${GAZEBO_INCLUDE_DIRS}
  src
  plugin
)

link_directories(
  ${GAZEBO_LIBRARY_DIRS}
)

add_library(d_drive_car SHARED
  plugin/d_drive_car_plugin.cpp
)

add_executable(d_drive_car_control
  src/d_drive_car_control.cpp
)

target_link_libraries(d_drive_car_control
  Middleware-Msgs::Geometry
  Middleware-Node::Middleware-Node
)

target_link_libraries(d_drive_car
  ${GAZEBO_LIBRARIES}
  Middleware-Msgs::Geometry
  Middleware-Node::Middleware-Node
)

