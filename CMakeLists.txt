cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

enable_language(CXX)

set(CMAKE_DEBUG_POSTFIX "-d")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF) # disable compiler extensions
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE Debug) # ensure CMAKE_BUILD_TYPE is not empty
endif()

# build version
set(build_version 0.3.0)

# project parameters
set(project_name forest)
set(${project_name}_version ${build_version})
set(cxx_standard cxx_std_17)

project(${project_name} VERSION "${${project_name}_version}")
set(${project_name}_soversion ${PROJECT_VERSION_MAJOR})
set(${project_name}_version_file_in cmake/${PROJECT_NAME}_version.hpp.in)
set(${project_name}_version_file_out ${PROJECT_NAME}_version.hpp)

function(configure_version_file)
  set(in "${CMAKE_CURRENT_SOURCE_DIR}/${${project_name}_version_file_in}")

  if(EXISTS "${in}")
    set(out "${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/${${project_name}_version_file_out}")
    message(STATUS "Configuring ${out}")
    configure_file("${in}" "${out}" @ONLY)
    source_group(TREE "${CMAKE_CURRENT_BINARY_DIR}" FILES "${out}")
  endif()
endfunction()

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

  message(STATUS "[${CMAKE_SYSTEM_PROCESSOR}] [${CMAKE_SYSTEM_NAME}] [${CMAKE_GENERATOR}] [${CMAKE_CXX_COMPILER_ID}] [${build_config}]")
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
get_target_property(sources ${PROJECT_NAME} SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

configure_version_file()

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
  install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets
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

  # install and export targets
  install(EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}-targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )

  # configure ${PROJECT_NAME}-config.cmake
  configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )

  # install ${PROJECT_NAME}-config.cmake
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )

  # configure ${PROJECT_NAME}-config-version.cmake
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    VERSION ${build_version}
    COMPATIBILITY AnyNewerVersion
  )

  # install ${PROJECT_NAME}-config-version.cmake
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )
endif()
