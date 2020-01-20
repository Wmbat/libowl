cmake_minimum_required( VERSION 3.15 )
project( EML
   VERSION 0.0.1
   DESCRIPTION "A memory management library"
   LANGUAGES CXX
)

option( EML_BUILD_TESTS "Build EML's unit tests" OFF )

if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release )
endif( )

if ( CMAKE_BUILD_TYPE EQUAL Release )
   message( STATUS "Building EML in release mode" )
else ( )
   message( STATUS "Building EML in debug mode" )
endif ( )

set( CLANG_DEBUG_FLAGS "-o0 -g -Wall -Wextra -Weverything -Werror" )
set( CLANG_RELEASE_FLAGS "-o3" )

if ( EML_BUILD_TESTS )
   enable_testing( )

   add_subdirectory( ../external/googletest build )
   add_subdirectory( tests )
endif ( )

add_library( EML STATIC )

set_target_properties( EML
   PROPERTIES
      CXX_EXTENSIONS OFF
)

target_compile_features( EML
   PRIVATE
      cxx_std_20
)

target_compile_options( EML
   PRIVATE
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:${CLANG_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:${CLANG_RELEASE_FLAGS}>
)
       
target_include_directories( EML
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_sources( EML
   PRIVATE
      "source/EML/memory_manager.cpp"
)
