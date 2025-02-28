cmake_minimum_required(VERSION 3.16)
project(Middleware-Master)
#sudo setcap cap_sys_ptrace+ep
set(CMAKE_CXX_STANDARD 20)

find_package(spdlog REQUIRED)


include_directories(src)

add_executable(${PROJECT_NAME} src/main.cpp
        src/core/Acceptor.hpp
        src/core/Acceptor.cpp
        src/core/Server.hpp
        src/core/Server.cpp
        src/core/Subscribe.cpp
        src/core/Subscribe.hpp
        src/core/Publish.cpp
        src/core/Publish.hpp
        src/core/MiddlewareManager.cpp
        src/core/MiddlewareManager.hpp
        src/core/Node.cpp
        src/core/Node.hpp
        src/core/Define.hpp
        src/core/UdsRouter.cpp
        src/core/UdsRouter.hpp
)

target_link_libraries(${PROJECT_NAME} spdlog::spdlog rt)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND cat  ~/.local/passwd | sudo -S setcap cap_sys_ptrace+ep ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
    COMMENT "运行ptrace附加到其他进程，使master创建其他进程的eventfd副本"
)