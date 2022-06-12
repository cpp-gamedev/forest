cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(forest-example)

if(NOT TARGET forest)
  find_package(forest REQUIRED CONFIG)
endif()

add_executable(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE forest::forest)
target_sources(${PROJECT_NAME} PRIVATE forest-example.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES forest-example.cpp)

if(CMAKE_CXX_COMPILER_ID MATCHES "^Clang$" OR CMAKE_CXX_COMPILER_ID MATCHES "^GNU$")
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
