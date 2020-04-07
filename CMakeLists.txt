cmake_minimum_required( VERSION 3.12 )
project( Epona
   VERSION 0.0.2
   DESCRIPTION "A Vulkan based rendering engine"
   LANGUAGES CXX 
)

option( BUILD_TESTS "Build unit tests" OFF )
option( BUILD_PERF_TESTS "Build performance tests" OFF )

if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release )
endif( )

message( STATUS "Compiling Epona with ${CMAKE_CXX_COMPILER_ID}" )

if ( BUILD_TESTS )
   set( EML_BUILD_TESTS ON CACHE BOOL "" FORCE )
endif ( )

if ( BUILD_PERF_TESTS )
   set( EML_BUILD_PERF_TESTS ON CACHE BOOL "" FORCE )
endif( )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE )

# nlohmann settings
set( JSON_BuildTests OFF CACHE BOOL "" FORCE )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake" )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( CLANG_DEBUG_FLAGS "-o0 -g -Weverything -Werror" )
set( CLANG_RELEASE_FLAGS "-o3" )
set( GNU_DEBUG_FLAGS "-o0, -g -Wall -Wextra -Werror" )
set( GNU_RELEASE_FLAGS "-o3" )

add_subdirectory( external/nlohmann )
add_subdirectory( external/glm )

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
      "source/main.cpp"
)
