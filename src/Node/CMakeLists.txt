cmake_minimum_required(VERSION 3.16)
project(Middleware-Node)

set(CMAKE_CXX_STANDARD 20)

find_package(spdlog REQUIRED)
# find_package(Protobuf REQUIRED)

add_library(${PROJECT_NAME} SHARED
        src/Node.cpp
        include/hmw/Node.hpp
        include/hmw/Publisher.decl.hpp
        include/hmw/Publisher.hpp
        include/hmw/PublisherInterface.hpp
        include/hmw/SubscriberInterface.hpp
        include/hmw/Subscriber.decl.hpp
        include/hmw/Subscriber.hpp
)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} spdlog::spdlog rt)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
export(TARGETS ${PROJECT_NAME} FILE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Targets.cmake NAMESPACE ${PROJECT_NAME}::)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    @ONLY)


# install(TARGETS ${PROJECT_NAME}
#     EXPORT ${PROJECT_NAME}Targets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
#     INCLUDES DESTINATION include
# )

# install(DIRECTORY include/ DESTINATION include/)

# install(EXPORT ${PROJECT_NAME}Targets
#     FILE ${PROJECT_NAME}Targets.cmake
#     NAMESPACE ${PROJECT_NAME}::
#     DESTINATION lib/cmake/${PROJECT_NAME}
# )

# include(CMakePackageConfigHelpers)
# configure_package_config_file(
#     cmake/${PROJECT_NAME}Config.cmake.in
#     ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
#     INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
# )

# install(FILES
#     ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
#     DESTINATION lib/cmake/${PROJECT_NAME}
# )



