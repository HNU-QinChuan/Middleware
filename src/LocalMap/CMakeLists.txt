cmake_minimum_required(VERSION 3.16)
project(local_map)

set(CMAKE_CXX_STANDARD 17)

find_package(spdlog REQUIRED)
find_package(PCL  REQUIRED)
find_package(Middleware-Node REQUIRED)
find_package(Middleware-Msgs REQUIRED)


aux_source_directory(src src)
add_executable(localmap ${src})
target_link_libraries(localmap
  ${PCL_LIBRARIES}
  Middleware-Node::Middleware-Node
  Middleware-Msgs::Geometry
  Middleware-Msgs::Nav
  Middleware-Msgs::Sensor
)