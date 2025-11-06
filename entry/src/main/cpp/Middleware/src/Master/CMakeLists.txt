cmake_minimum_required(VERSION 3.16)
project(Middleware-Master)
#sudo setcap cap_sys_ptrace+ep
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=1)

#set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(OHOS_SDK_PATH "D:/study/y1/openharmony/ohos-sdk/11")
set(CMAKE_SYSROOT "${OHOS_SDK_PATH}/native/sysroot")
set(CLANG_PATH "${OHOS_SDK_PATH}/native/llvm/bin/clang++")
set(CMAKE_TOOLCHAIN_FILE "${OHOS_SDK_PATH}/native/build/cmake/ohos.toolchain.cmake")
set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined -Wl,--as-needed")

set(OHOS_ARCH "arm64-v8a")

add_compile_definitions(BOOST_ASIO_HAS_STD_INVOKE_RESULT _LIBCPP_ENABLE_CXX17_REMOVED_FEATURES)
add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)

find_package(spdlog REQUIRED)

include_directories(src)

aux_source_directory(src/shm shm)
aux_source_directory(src/shm/controller shm_controller)
aux_source_directory(src/interface interface)
aux_source_directory(src/interface/controller interface_controller)
aux_source_directory(src/tcp tcp)


add_executable(${PROJECT_NAME} src/main.cpp
    src/napi_master.cpp
    src/napi_master.h
    ${shm}
    ${shm_controller}
    ${interface}
    ${interface_controller}
    ${tcp}
    src/MiddlewareManager.cpp
    src/MiddlewareManager.hpp
    src/InterfaceManager.cpp
    src/InterfaceManager.hpp
    src/ProcessManager.cpp
    src/ProcessManager.hpp
)

target_link_libraries(${PROJECT_NAME}
                    spdlog::spdlog rt jsoncpp.a
                    librawfile.z.so
 #                   -Wl,-Bstatic
  #                  c++ c++abi
                    #libhilog_ndk.z.so
                    )

#install config/masterconfig.json到CMAKE_RUNTIME_OUTPUT_DIRECTORY
#不调用install，而是在构建后
#[[
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/config/masterconfig.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/masterconfig.json
    COMMENT "拷贝配置文件到输出目录"
)


add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND cat  ~/.local/passwd | sudo -S setcap cap_sys_ptrace+ep ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}
    COMMENT "运行ptrace附加到其他进程，使master创建其他进程的eventfd副本"
)
]]

# 拷贝配置文件到可执行目录
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/config/masterconfig.json
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/masterconfig.json
    COMMENT "Copying config file to output directory"
)
