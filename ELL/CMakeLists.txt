cmake_minimum_required( VERSION 3.12 )
project( ELL
   VERSION 0.0.1
   DESCRIPTION "A Logging library"
   LANGUAGES CXX
)

if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release CACHE FORCE )
endif( )

message( STATUS "Build ELL: 0.0.1" )

if ( CMAKE_BUILD_TYPE MATCHES Release )
   message( STATUS "Build type: Release" )
else ( )
   message( STATUS "Build type: Debug" )
endif ( )

set( CLANG_DEBUG_FLAGS "-o0 -g -Wall -Wextra -Weverything -Werror" )
set( CLANG_RELEASE_FLAGS "-o3" )
set( GNU_DEBUG_FLAGS "-o0, -g -Wall -Wextra -Werror" )
set( GNU_RELEASE_FLAGS "-o3" )

add_subdirectory( external/spdlog )

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

      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:${GNU_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:${GNU_RELEASE_FLAGS}>
)
       
target_include_directories( ELL
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries( ELL
   PUBLIC
      spdlog_header_only
)

target_sources( ELL
   PRIVATE
      "source/ELL/logger.cpp"
)
