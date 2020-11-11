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

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_BENCH "Build benchmarks" ON)

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

if (BUILD_TESTS)
   set(VERMILLON_CORE_BUILD_TESTS ON CACHE BOOL "" FORCE)
   set(VERMILLON_UTIL_BUILD_TESTS ON CACHE BOOL "" FORCE)
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(VERMILLON_CORE_STATIC ON CACHE BOOL "" FORCE)

message(STATUS "[${PROJECT_NAME}] Building unit tests: ${BUILD_TESTS}")
message(STATUS "[${PROJECT_NAME}] Building benchmarks: ${BUILD_BENCH}")

add_subdirectory(modules/gfx)
add_subdirectory(modules/ui)
add_subdirectory(modules/util)
add_subdirectory(modules/vkn)

#add_subdirectory(samples/playground)
add_subdirectory(samples/water_simulation)
