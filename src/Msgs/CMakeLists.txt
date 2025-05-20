cmake_minimum_required(VERSION 3.16)
project(Middleware-Msgs)

set(CMAKE_CXX_STANDARD 17)

# 查找并检查 Protobuf
find_package(Protobuf REQUIRED)
if(NOT Protobuf_FOUND)
    message(FATAL_ERROR "Protobuf not found!")
endif()

# 设置proto根目录 - 添加这一行
set(PROTO_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)

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

    # 遍历每个 .proto 文件，记录生成的文件名
    foreach(PROTO_FILE ${PROTO_FILES})
        get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
        list(APPEND PROTO_SRCS "${GENERATED_DIR}/${PROTO_NAME}.pb.cc")
        list(APPEND PROTO_HDRS "${GENERATED_DIR}/${PROTO_NAME}.pb.h")
    endforeach()

    # 创建一个标记文件，表示所有proto文件的生成
    set(GENERATION_MARKER "${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}_proto_generation_marker")

    # 一次性生成所有文件，处理内部依赖
    add_custom_command(
            OUTPUT ${GENERATION_MARKER} ${PROTO_SRCS} ${PROTO_HDRS}
            COMMAND protobuf::protoc
            ARGS --proto_path=${PROTO_ROOT_DIR} --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/src ${PROTO_FILES}
            COMMAND ${CMAKE_COMMAND} -E touch ${GENERATION_MARKER}
            DEPENDS ${PROTO_FILES}
            COMMENT "Generating protobuf files for ${LIB_NAME}"
            VERBATIM
    )

    # 将生成的文件添加到库中
    add_library(${LIB_NAME} SHARED ${PROTO_SRCS} ${PROTO_HDRS})
    target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(${LIB_NAME} PUBLIC ${Protobuf_LIBRARIES})

    # 设置别名
    add_library(${PROJECT_NAME}::${LIB_NAME} ALIAS ${LIB_NAME})
endfunction()

# 生成 Std 库
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto/Std)
set(GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/Std)
generate_protobuf_library(${PROTO_DIR} ${GENERATED_DIR} Std)

# 生成 BuiltinInterfaces 库
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto/BuiltinInterfaces)
set(GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/BuiltinInterfaces)
generate_protobuf_library(${PROTO_DIR} ${GENERATED_DIR} BuiltinInterfaces)

# 生成 Dgps 库及其依赖
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto/Dgps)
set(GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/Dgps)
generate_protobuf_library(${PROTO_DIR} ${GENERATED_DIR} Dgps)
add_dependencies(Dgps Std)  # Dgps 依赖 Std
target_link_libraries(Dgps PUBLIC Std)  # Dgps 链接 Std

# 生成 Geometry 库及其依赖
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto/Geometry)
set(GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/Geometry)
generate_protobuf_library(${PROTO_DIR} ${GENERATED_DIR} Geometry)
add_dependencies(Geometry Std)  # Geometry 依赖 Std
target_link_libraries(Geometry PUBLIC Std)  # Geometry 链接 Std

# 生成 Nav 库及其依赖
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto/Nav)
set(GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/Nav)
generate_protobuf_library(${PROTO_DIR} ${GENERATED_DIR} Nav)
add_dependencies(Nav Std BuiltinInterfaces)
target_link_libraries(Nav PUBLIC Std BuiltinInterfaces Geometry)

# 生成 TaskPlanner 库及其依赖
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto/TaskPlanner)
set(GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/TaskPlanner)
generate_protobuf_library(${PROTO_DIR} ${GENERATED_DIR} TaskPlanner)
add_dependencies(TaskPlanner Geometry)
target_link_libraries(TaskPlanner PUBLIC Geometry)

# 导出配置文件
export(TARGETS Std FILE ${CMAKE_BINARY_DIR}/StdTargets.cmake NAMESPACE ${PROJECT_NAME}::)
export(TARGETS Dgps FILE ${CMAKE_BINARY_DIR}/DgpsTargets.cmake NAMESPACE ${PROJECT_NAME}::)
export(TARGETS Geometry FILE ${CMAKE_BINARY_DIR}/GeometryTargets.cmake NAMESPACE ${PROJECT_NAME}::)
export(TARGETS Nav FILE ${CMAKE_BINARY_DIR}/NavTargets.cmake NAMESPACE ${PROJECT_NAME}::)
export(TARGETS BuiltinInterfaces FILE ${CMAKE_BINARY_DIR}/BuiltinInterfaces.cmake NAMESPACE ${PROJECT_NAME}::)
export(TARGETS TaskPlanner FILE ${CMAKE_BINARY_DIR}/TaskPlanner.cmake NAMESPACE ${PROJECT_NAME}::)

# 配置文件
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        @ONLY)