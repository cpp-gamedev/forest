cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

enable_language(CXX)

set(CMAKE_DEBUG_POSTFIX "-d")
set(CMAKE_CXX_EXTENSIONS OFF) # disable compiler extensions
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE Debug) # ensure CMAKE_BUILD_TYPE is not empty
endif()

# build version
set(build_version 0.2.0)

# project parameters
set(project_name forest)
set(${project_name}_version ${build_version})
set(cxx_standard cxx_std_17)

project(${project_name} VERSION "${${project_name}_version}")
set(${project_name}_soversion ${PROJECT_VERSION_MAJOR})
set(is_root_project OFF) # indicate if this is the top-level project

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(is_root_project ON)
  message(STATUS "${PROJECT_NAME} is top level")
endif()

# options
option(FOREST_BUILD_EXAMPLE "Build forest example" ${is_root_project})
option(FOREST_INSTALL "Install forest" ${is_root_project})

# includes
include(CheckPIESupported)
check_pie_supported()

# header text
if(is_root_project)
  set(build_config ${CMAKE_BUILD_TYPE})

  if(CMAKE_CONFIGURATION_TYPES)
    set(build_config "Multi-config")
  endif()

  message(STATUS "[${CMAKE_SYSTEM_PROCESSOR}] [${PLATFORM}] [${CMAKE_GENERATOR}] [${CMAKE_CXX_COMPILER_ID}] [${build_config}]")
endif()

# compile commands
if(is_root_project)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  message(STATUS "Exporting compile commands")
endif()

# library
add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE include/forest/forest.hpp)
target_compile_features(${PROJECT_NAME} INTERFACE ${cxx_standard})
target_include_directories(${PROJECT_NAME} INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# examples
if(FOREST_BUILD_EXAMPLE)
  message(STATUS "Adding forest example to build tree")
  add_subdirectory(example)

  enable_testing()
  message(STATUS "Adding forest test target")
  add_test(NAME forest-test COMMAND forest-example)
endif()

if(FOREST_INSTALL)
  include(GNUInstallDirs)
  include(CMakePackageConfigHelpers)

  # install targets
  install(TARGETS forest EXPORT forest-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  # install headers from include
  install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
  )

  # install generated headers
  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
  )

  # install export
  install(EXPORT forest-targets
    FILE forest-targets.cmake
    NAMESPACE forest::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/forest
  )

  # configure package config
  configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/forest-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/forest
  )

  # install forest-config.cmake
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/forest-config.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/forest
  )

  # configure forest-version.cmake
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/forest-config-version.cmake"
    VERSION ${build_version}
    COMPATIBILITY AnyNewerVersion
  )

  # install forest-version.cmake
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/forest-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/forest
  )
endif()
