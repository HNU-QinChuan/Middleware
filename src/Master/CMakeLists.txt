cmake_minimum_required(VERSION 3.16)
project(Middleware-Master)
#sudo setcap cap_sys_ptrace+ep
set(CMAKE_CXX_STANDARD 20)

find_package(spdlog REQUIRED)


include_directories(src)

aux_source_directory(src/shm shm)
aux_source_directory(src/shm/controller shm_controller)


add_executable(${PROJECT_NAME} src/main.cpp
    ${shm}
    ${shm_controller}
    src/MiddlewareManager.cpp
    src/MiddlewareManager.hpp
    src/InterfaceManager.cpp
    src/InterfaceManager.hpp

)

target_link_libraries(${PROJECT_NAME} spdlog::spdlog rt jsoncpp)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND cat  ~/.local/passwd | sudo -S setcap cap_sys_ptrace+ep ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}
    COMMENT "运行ptrace附加到其他进程，使master创建其他进程的eventfd副本"
)