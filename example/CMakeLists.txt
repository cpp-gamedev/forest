add_executable(forest-example)
target_link_libraries(forest-example PRIVATE forest::forest)
target_sources(forest-example PRIVATE forest-example.cpp)

if(CMAKE_CXX_COMPILER_ID MATCHES "^Clang$" OR CMAKE_CXX_COMPILER_ID MATCHES "^GNU$")
  target_compile_options(forest-example PRIVATE -Wall -Wextra -Wpedantic)
endif()
