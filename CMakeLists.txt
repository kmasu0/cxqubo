cmake_minimum_required(VERSION 3.14.3)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(CXQUBO_BUILT_STANDALONE TRUE)
endif()

set(CXQUBO_VERSION_MAJOR 0 CACHE STRING "major version" FORCE)
set(CXQUBO_VERSION_MINOR 1 CACHE STRING "minor version" FORCE)
set(CXQUBO_VERSION_PATCH 0 CACHE STRING "patch version" FORCE)
set(CXQUBO_VERSION_FULL ${CXQUBO_VERSION_MAJOR}.${CXQUBO_VERSION_MINOR}.${CXQUBO_VERSION_PATCH}
  CACHE STRING "full version" FORCE)

project(cxqubo
  VERSION ${CXQUBO_VERSION_FULL}
  LANGUAGES C CXX ASM
  )
include(GNUInstallDirs)

option(CXQUBO_ENABLE_UNITTESTS "Build and run the cxqubo example program." ON)
option(CXQUBO_BUILD_EXAMPLE "Build the cxqubo example program." OFF)
option(CXQUBO_ENABLE_DUMP "Enable dump method even if release build." OFF)

if (CXQUBO_BUILT_STANDALONE)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

list(APPEND CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
  )

include(AddBuildTarget)
include(CompileOptions)
include(CheckLibraryExists)

include(external/cimod.cmake)
# include(external/fmt.cmake)

include_directories(
  ${CXQUBO_INCLUDE_DIRECTORY}
  ${CXQUBO_SOURCE_INCLUDE_DIRECTORY}
  )

configure_file(
  ${CXQUBO_SOURCE_INCLUDE_DIRECTORY}/cxqubo/config/cxqubo-config.h.cmake
  ${CXQUBO_INCLUDE_DIRECTORY}/cxqubo/config/cxqubo-config.h)

add_subdirectory(include)
if (CXQUBO_ENABLE_UNITTESTS)
  enable_testing()
  add_subdirectory(unittests)
endif()
if (CXQUBO_BUILD_EXAMPLE)
  add_subdirectory(examples)
endif()

add_subdirectory(cmake/modules)

install(DIRECTORY ${CXQUBO_INCLUDE_DIRECTORY}/cxqubo
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT cxqubo_header_only
  FILES_MATCHING
  PATTERN "*.h"
  PATTERN "CMakeFiles" EXCLUDE
)

install(DIRECTORY ${CXQUBO_SOURCE_INCLUDE_DIRECTORY}/cxqubo
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT cxqubo_header_only
  FILES_MATCHING
  PATTERN "*.h"
  PATTERN "CMakeFiles" EXCLUDE
)

