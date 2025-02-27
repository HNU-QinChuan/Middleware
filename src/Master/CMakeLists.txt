cmake_minimum_required(VERSION 3.16)
project(Middleware-Master)
#sudo setcap cap_sys_ptrace+ep
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
        src/Define.hpp
)

target_link_libraries(${PROJECT_NAME} spdlog::spdlog rt)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND cat  ~/.local/passwd | sudo -S setcap cap_sys_ptrace+ep ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
    COMMENT "运行ptrace附加到其他进程，使master创建其他进程的eventfd副本"
)