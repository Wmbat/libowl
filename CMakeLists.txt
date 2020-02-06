cmake_minimum_required( VERSION 3.15 )
project( Epona
   VERSION 0.0.2
   DESCRIPTION "A Vulkan based rendering engine"
   LANGUAGES CXX 
)

cmake_policy(SET CMP0048 NEW)

if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release )
endif( )

message( STATUS "Compiling Epona with ${CMAKE_CXX_COMPILER_ID}" )

set( EML_BUILD_TESTS ON CACHE BOOL "" FORCE )

# nlohmann settings
set( JSON_BuildTests OFF CACHE BOOL "" FORCE )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake" )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( CLANG_DEBUG_FLAGS "-o0 -g -Wall -Wextra -Weverything -Werror" )
set( CLANG_RELEASE_FLAGS "-o3" )

add_subdirectory( external/nlohmann )
add_subdirectory( external/glm )

add_subdirectory( EML )
add_subdirectory( ELL )
add_subdirectory( EGL )
add_subdirectory( EWL )

add_executable( Epona )

set_target_properties( Epona
   PROPERTIES
      CXX_EXTENSIONS OFF
)

target_compile_features( Epona
   PRIVATE
      cxx_std_20
)

target_compile_options( Epona 
   PRIVATE
      $<$<PLATFORM_ID:UNIX>:-pthread>

      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:${CLANG_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:${CLANG_RELEASE_FLAGS}>
)

target_link_libraries( Epona
   PUBLIC
      nlohmann_json
      glm
      EML
      ELL
      EGL
      EWL
)

target_sources( Epona   
   PRIVATE
      "source/main.cpp"
)
