cmake_minimum_required(VERSION 3.16)
project(Middleware-Master)

set(CMAKE_CXX_STANDARD 20)

find_package(spdlog REQUIRED)


include_directories(include)

add_executable(${PROJECT_NAME} src/main.cpp
        src/Acceptor.hpp
        src/Acceptor.cpp
        src/Server.hpp
        src/Server.cpp
        src/Subscribe.cpp
        src/Subscribe.hpp
        src/Publish.cpp
        src/Publish.hpp
        src/MiddlewareManager.cpp
        src/MiddlewareManager.hpp
        src/Node.cpp
        src/Node.hpp
)

target_link_libraries(${PROJECT_NAME} spdlog::spdlog)