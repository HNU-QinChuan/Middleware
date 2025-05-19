cmake_minimum_required(VERSION 3.16)

project(Gazebo)

set(CMAKE_CXX_STANDARD 17)

find_package(spdlog REQUIRED)
find_package(Middleware-Node REQUIRED)
find_package(Middleware-Msgs REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(gazebo REQUIRED)
find_package(Qt5Core REQUIRED)

include_directories(
  ${GAZEBO_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  src
  plugin
)

link_directories(
  ${GAZEBO_LIBRARY_DIRS}
)

add_library(d_drive_car SHARED
  plugin/d_drive_car_plugin.cpp
)

add_library(gui_mouse_click_goal SHARED
  plugin/gui_mouse_click_goal.cpp
)

add_library(pose_publisher SHARED
  plugin/pose_publisher_plugin.cpp
)

add_library(odom_publisher_plugin SHARED
  plugin/odom_publisher_plugin.cpp
)

add_executable(d_drive_car_control
  src/d_drive_car_control.cpp
)

add_executable(gazebo_publisher
  test/publisher.cpp
)

add_executable(gazebo_subscriber
  test/subscriber.cpp
)

target_link_libraries(d_drive_car_control
  Middleware-Msgs::Geometry
  Middleware-Node::Middleware-Node
)

target_link_libraries(gui_mouse_click_goal
  ${GAZEBO_LIBRARIES}
  Qt5::Core
  Middleware-Msgs::Geometry
  Middleware-Node::Middleware-Node
)

target_link_libraries(d_drive_car
  ${GAZEBO_LIBRARIES}
  Middleware-Msgs::Geometry
  Middleware-Node::Middleware-Node
)

target_link_libraries(pose_publisher
  ${GAZEBO_LIBRARIES}
  Middleware-Msgs::Geometry
  Middleware-Node::Middleware-Node
)

target_link_libraries(gazebo_publisher
  Middleware-Msgs::Geometry
  Middleware-Node::Middleware-Node
)

target_link_libraries(gazebo_subscriber
  Middleware-Msgs::Geometry
  Middleware-Msgs::Nav
  Middleware-Node::Middleware-Node
)

target_link_libraries(odom_publisher_plugin
  Middleware-Msgs::Nav
  Middleware-Node::Middleware-Node
)