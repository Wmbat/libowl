cmake_minimum_required( VERSION 3.15 )
project( EWL
   VERSION 0.0.1
   DESCRIPTION "A Window management library"
   LANGUAGES CXX
)

if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release )
endif( )

if ( CMAKE_BUILD_TYPE EQUAL Release )
   message( STATUS "Building EWL in release mode" )
else ( )
   message( STATUS "Building EWL in debug mode" )
endif ( )

set( CLANG_DEBUG_FLAGS "-o0 -g -Wall -Wextra -Weverything -Werror" )
set( CLANG_RELEASE_FLAGS "-o3" )

add_library( EWL STATIC )

set_target_properties( EWL
   PROPERTIES
      CXX_EXTENSIONS OFF
)

target_compile_features( EWL
   PRIVATE
      cxx_std_17
)

target_compile_options( EWL
   PRIVATE
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:${CLANG_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:${CLANG_RELEASE_FLAGS}>
)

target_include_directories( EWL
   PUBLIC
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
   PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_sources( EWL
   PRIVATE
      "source/EWL/window.cpp"
)
