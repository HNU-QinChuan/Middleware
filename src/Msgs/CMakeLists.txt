cmake_minimum_required(VERSION 3.16)
project(Middleware-Msgs)

set(CMAKE_CXX_STANDARD 20)

# 查找并检查 Protobuf
find_package(Protobuf REQUIRED)
if(NOT Protobuf_FOUND)
    message(FATAL_ERROR "Protobuf not found!")
endif()

# 公共的 protobuf 处理函数
function(generate_protobuf_library PROTO_DIR GENERATED_DIR LIB_NAME)
    # 创建输出目录
    file(MAKE_DIRECTORY ${GENERATED_DIR})

    # 查找所有 .proto 文件
    file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")
    if(NOT PROTO_FILES)
        message(FATAL_ERROR "Error: No .proto files found in ${PROTO_DIR}")
    endif()

    # 初始化 .pb.cc 和 .pb.h 列表
    set(PROTO_SRCS "")
    set(PROTO_HDRS "")

    # 遍历每个 .proto 文件，生成对应的 .cc 和 .h 文件
    foreach(PROTO_FILE ${PROTO_FILES})
        get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
        list(APPEND PROTO_SRCS "${GENERATED_DIR}/${PROTO_NAME}.pb.cc")
        list(APPEND PROTO_HDRS "${GENERATED_DIR}/${PROTO_NAME}.pb.h")

        add_custom_command(
                OUTPUT ${GENERATED_DIR}/${PROTO_NAME}.pb.cc ${GENERATED_DIR}/${PROTO_NAME}.pb.h
                COMMAND protobuf::protoc
                ARGS --proto_path=${PROTO_DIR} --cpp_out=${GENERATED_DIR} ${PROTO_FILE}
                DEPENDS ${PROTO_FILE}
                COMMENT "Generating ${PROTO_NAME}.pb.cc and ${PROTO_NAME}.pb.h"
                VERBATIM
        )
    endforeach()

    # 将生成的 .pb.cc 和 .pb.h 文件添加到库中
    set(LIB_SOURCES ${PROTO_SRCS} ${PROTO_HDRS})

    # 创建库
    add_library(${LIB_NAME} SHARED ${LIB_SOURCES})
    target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

    # 设置别名
    add_library(${PROJECT_NAME}::${LIB_NAME} ALIAS ${LIB_NAME})
endfunction()

# 生成 Dgps 库
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto/Dgps)
set(GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/Dgps)
generate_protobuf_library(${PROTO_DIR} ${GENERATED_DIR} Dgps)

# 生成 Std 库
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto/Std)
set(GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/Std)
generate_protobuf_library(${PROTO_DIR} ${GENERATED_DIR} Std)

# 导出配置文件
export(TARGETS Std FILE ${CMAKE_BINARY_DIR}/StdTargets.cmake NAMESPACE ${PROJECT_NAME}::)
export(TARGETS Dgps FILE ${CMAKE_BINARY_DIR}/DgpsTargets.cmake NAMESPACE ${PROJECT_NAME}::)

# 配置文件
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        @ONLY)
