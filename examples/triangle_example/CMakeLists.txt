cmake_minimum_required( VERSION 3.15 )
project( triangle_example
   VERSION 0.0.1
   DESCRIPTION "A simple example of the UVE engine to render a triangle"
   LANGUAGES CXX 
)

if ( NOT CMAKE_BUILD_TYPE )
   set ( CMAKE_BUILD_TYPE Release )
endif( )

set( CLANG_DEBUG_FLAGS "-o0 -g -Wall -Wextra -Weverything -Werror" )
set( CLANG_RELEASE_FLAGS "-o3" )

add_executable( triangle_example )

set_target_properties( triangle_example
   PROPERTIES
      CXX_EXTENSIONS OFF
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples/triangle/bin"
)

target_compile_features( triangle_example
   PRIVATE
      cxx_std_17
)

target_compile_options( triangle_example 
   PRIVATE
      $<$<PLATFORM_ID:UNIX>:-pthread>

      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:${CLANG_DEBUG_FLAGS}>

      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:${CLANG_RELEASE_FLAGS}>
)

target_link_libraries( triangle_example
   PRIVATE
      UVE
)

target_sources( triangle_example
   PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)
