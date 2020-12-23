# CMake project initialization

cmake_minimum_required( VERSION 3.14 FATAL_ERROR )

# Download CPM.cmake

set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
set(CPM_DOWNLOAD_VERSION 0.27)

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION} AND CPM_VERSION STREQUAL CPM_DOWNLOAD_VERSION))
    message(STATUS "Downloading CPM.cmake")
    file(DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif()

include(${CPM_DOWNLOAD_LOCATION})

# Set the project language toolchain, version and description

project(vermillon
    VERSION 0.0.1
    DESCRIPTION "A Vulkan based rendering engine"
    LANGUAGES CXX
)

# User interface declarations

option(VERMILLON_BUILD_TESTS "Build unit tests" OFF)
option(VERMILLON_SANITIZE_ADDRESS "Sanitize address" OFF)
option(VERMILLON_ENFORCE_CONTRACTS "Enforce all contracts at run-time" OFF)
option(VERMILLON_SHOW_DEBUG_LOGGING "Show logs labelled as debug" ON)
option(VERMILLON_BUILD_SAMPLES "Build the sample projects" ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "[${PROJECT_NAME}] Compiling with ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "[${PROJECT_NAME}] ${PROJECT_VERSION}")

if (CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "[${PROJECT_NAME}] Build type: Release")
else ()
    message(STATUS "[${PROJECT_NAME}] Build type: Debug")
endif ()

if (VERMILLON_BUILD_TESTS)
   set(VERMILLON_CORE_BUILD_TESTS ON CACHE BOOL "" FORCE)
   set(VERMILLON_GFX_BUILD_TESTS ON CACHE BOOL "" FORCE)
   set(VERMILLON_UTIL_BUILD_TESTS ON CACHE BOOL "" FORCE)
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(VERMILLON_CORE_STATIC ON CACHE BOOL "" FORCE)

message(STATUS "[${PROJECT_NAME}] Building unit tests: ${VERMILLON_BUILD_TESTS}")
message(STATUS "[${PROJECT_NAME}] Building with address sanitizer: ${VERMILLON_SANITIZE_ADDRESS}")
message(STATUS "[${PROJECT_NAME}] Building provided samples: ${VERMILLON_BUILD_SAMPLES}")
message(STATUS "[${PROJECT_NAME}] Enforce all contracts at run-time: ${VERMILLON_ENFORCE_CONTRACTS}")
message(STATUS "[${PROJECT_NAME}] Enable display of debug level logs: ${VERMILLON_SHOW_DEBUG_LOGGING}")

add_subdirectory(vermillon)

if (VERMILLON_BUILD_SAMPLES)
    #add_subdirectory(samples/playground)
    add_subdirectory(samples/fluid_simulation)
endif ()
