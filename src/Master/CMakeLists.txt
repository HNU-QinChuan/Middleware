cmake_minimum_required(VERSION 3.16)
project(Middleware-Master)
#sudo setcap cap_sys_ptrace+ep
set(CMAKE_CXX_STANDARD 20)

find_package(spdlog REQUIRED)


include_directories(src)

add_executable(${PROJECT_NAME} src/main.cpp
        src/shm/Acceptor.hpp
        src/shm/Acceptor.cpp
        src/shm/Server.hpp
        src/shm/Server.cpp
        src/shm/Subscribe.cpp
        src/shm/Subscribe.hpp
        src/shm/Publish.cpp
        src/shm/Publish.hpp
        src/MiddlewareManager.cpp
        src/MiddlewareManager.hpp
        src/shm/Node.cpp
        src/shm/Node.hpp
        src/shm/Define.hpp
        src/shm/UdsRouter.cpp
        src/shm/UdsRouter.hpp
        src/shm/controller/NodeController.cpp
        src/shm/controller/NodePubController.cpp
        src/shm/controller/NodeSubController.cpp
)

target_link_libraries(${PROJECT_NAME} spdlog::spdlog rt jsoncpp)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND cat  ~/.local/passwd | sudo -S setcap cap_sys_ptrace+ep ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}
    COMMENT "运行ptrace附加到其他进程，使master创建其他进程的eventfd副本"
)