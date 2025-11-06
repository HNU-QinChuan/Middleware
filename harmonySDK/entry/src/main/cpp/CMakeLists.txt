# the minimum version of CMake.
cmake_minimum_required(VERSION 3.16)
project(Mid_napi)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=1)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(OHOS_SDK_PATH "D:/study/y1/openharmony/ohos-sdk/11")
set(CMAKE_SYSROOT "${OHOS_SDK_PATH}/native/sysroot")
set(CLANG_PATH "${OHOS_SDK_PATH}/native/llvm/bin/clang++")
set(CMAKE_TOOLCHAIN_FILE "${OHOS_SDK_PATH}/native/build/cmake/ohos.toolchain.cmake")
set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined -Wl,--as-needed")

set(OHOS_ARCH "arm64-v8a")

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()

include_directories(${NATIVERENDER_ROOT_PATH}
                    ${NATIVERENDER_ROOT_PATH}/include
                    ${OHOS_SDK_PATH}/native/sysroot/usr/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/Middleware
                    ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/src/Node/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/src/Util/src
                    ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/src/Msgs/src
                    )

#链接库
link_directories(
    ${OHOS_SDK_PATH}/native/sysroot/usr/lib/aarch64-linux-ohos
    ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/src/Node/build/lib
)

add_compile_definitions(BOOST_ASIO_HAS_STD_INVOKE_RESULT _LIBCPP_ENABLE_CXX17_REMOVED_FEATURES)
add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)

# 递归查找所有 .pb.cc 文件
file(GLOB_RECURSE PROTO_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Middleware/src/Msgs/src/*.pb.cc"
)

set(MIDDLEWARE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/napi_init.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/src/Util/src/napi_util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/src/Node/src/Node.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/src/Node/src/NodeImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/src/Node/src/Timer.cpp
    ${PROTO_SRCS}
)

add_library(entry SHARED
    ${MIDDLEWARE_SRC}
)

#target_compile_options(entry PRIVATE "-stdlib=libc++")

target_link_libraries(entry PUBLIC libace_napi.z.so
                        libhilog_ndk.z.so
                        librawfile.z.so
                        pthread
                        jsoncpp.a
                        protobuf.a
                        spdlog
                        rt
                        yaml-cpp
                        boost_system.a
                        boost_thread.a
                        uv
                        #middleware_node_shm
                        #middleware_node_hmw
                        )

target_link_options(entry PRIVATE "-Wl,--no-as-needed")