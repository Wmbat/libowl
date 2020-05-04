cmake_minimum_required( VERSION 3.12 )

include( FetchContent )
include(ExternalProject)
include( "cmake/HunterGate.cmake" )

HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.253.tar.gz"
    SHA1 "88ea6d37c897a81a080eb9ae0f69d7807bbb3c73"
)

project( Epona
   VERSION 0.0.2
   DESCRIPTION "A Vulkan based rendering engine"
   LANGUAGES CXX 
)

option( BUILD_TESTS "Build unit tests" OFF )
option( BUILD_BENCH "Build benchmarks" OFF )

if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release )
endif( )

message( STATUS "[${PROJECT_NAME}] Compiling with ${CMAKE_CXX_COMPILER_ID}" )

if ( BUILD_TESTS )
    set( ESL_BUILD_TESTS ON CACHE BOOL "" FORCE )
endif ( )

if ( BUILD_BENCH )
    set( ESL_BUILD_BENCH ON CACHE BOOL "" FORCE )
endif( )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE )

# nlohmann settings
set( JSON_BuildTests OFF CACHE BOOL "" FORCE )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake" )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( CLANG_DEBUG_FLAGS -o0 -Wall -Wextra -Werror )
set( CLANG_RELEASE_FLAGS -o3 )
set( GNU_DEBUG_FLAGS -o0, -g -Wall -Wextra -Werror )
set( GNU_RELEASE_FLAGS -o3 )

hunter_add_package( glm )
hunter_add_package( GTest )
hunter_add_package( nlohmann_json )

find_package( glm REQUIRED )
find_package( GTest CONFIG REQUIRED )
find_package( nlohmann_json CONFIG REQUIRED )

add_subdirectory( ESL )
add_subdirectory( EGL )

add_executable( ${PROJECT_NAME} )

set_target_properties( ${PROJECT_NAME}
   PROPERTIES
      CXX_EXTENSIONS OFF
)

target_compile_features( ${PROJECT_NAME}
   PRIVATE
      cxx_std_20
)

target_compile_options( ${PROJECT_NAME}
   PRIVATE
      $<$<PLATFORM_ID:UNIX>:-pthread>

      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:${CLANG_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:${CLANG_RELEASE_FLAGS}>

      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:${GNU_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:${GNU_RELEASE_FLAGS}>
)

target_link_libraries( ${PROJECT_NAME}
   PUBLIC
      nlohmann_json
      glm
      EGL
)

target_sources( ${PROJECT_NAME}
   PRIVATE
      source/main.cpp
)
