cmake_minimum_required( VERSION 3.14 FATAL_ERROR )
include( FetchContent )
project( epona
   VERSION 0.0.2
   DESCRIPTION "A Vulkan based rendering engine"
   LANGUAGES CXX 
)

option( BUILD_TESTS "Build unit tests" ON )
option( BUILD_BENCH "Build benchmarks" ON )

if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release )
endif( )

message( STATUS "[${PROJECT_NAME}] Compiling with ${CMAKE_CXX_COMPILER_ID}" )

set( EPONA_CORE_STATIC ON CACHE BOOL "" FORCE )

if ( BUILD_TESTS )
   set( EPONA_CORE_BUILD_TESTS ON CACHE BOOL "" FORCE )
endif ( )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake" )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( CLANG_DEBUG_FLAGS -o0 -Wall -Wextra -Werror )
set( CLANG_RELEASE_FLAGS -o3 )
set( GNU_DEBUG_FLAGS -o0, -g -Wall -Wextra -Werror )
set( GNU_RELEASE_FLAGS -o3 )

message( STATUS "[${PROJECT_NAME}] Building unit tests: ${BUILD_TESTS}" )
message( STATUS "[${PROJECT_NAME}] Building benchmarks: ${BUILD_BENCH}" )

include( CMakeLists.txt.in )

add_subdirectory( epona_core )

add_executable( ${PROJECT_NAME} )

set_target_properties( ${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF )

target_compile_features( ${PROJECT_NAME} PRIVATE cxx_std_20 )

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
      glm
      epona_core
)

target_sources( ${PROJECT_NAME}
   PRIVATE
      source/main.cpp
)
