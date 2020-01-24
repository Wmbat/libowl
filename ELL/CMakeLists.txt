cmake_minimum_required( VERSION 3.15 )
project( ELL
   VERSION 0.0.1
   DESCRIPTION "A Logging library"
   LANGUAGES CXX
)

if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release )
endif( )

if ( CMAKE_BUILD_TYPE EQUAL Release )
   message( STATUS "Building ELL version 0.0.1 in release mode" )
else ( )
   message( STATUS "Building EML version 0.0.1 in debug mode" )
endif ( )

set( CLANG_DEBUG_FLAGS "-o0 -g -Wall -Wextra -Weverything -Werror" )
set( CLANG_RELEASE_FLAGS "-o3" )

add_library( ELL STATIC )

set_target_properties( ELL
   PROPERTIES
      CXX_EXTENSIONS OFF
)

target_compile_features( ELL
   PRIVATE
      cxx_std_20
)

target_compile_options( ELL
   PRIVATE
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:${CLANG_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:${CLANG_RELEASE_FLAGS}>
)
       
target_include_directories( ELL
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_sources( ELL
   PRIVATE
      "source/ELL/logger.cpp"
)
