cmake_minimum_required( VERSION 3.15 )
project( EML
   VERSION 0.1.1
   DESCRIPTION "A memory management library"
   LANGUAGES CXX
)

option( EML_BUILD_TESTS "Build EML's unit tests" OFF )

if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release CACHE FORCE )
endif( )

message( STATUS "Build ${PROJECT_NAME}: ${PROJECT_VERSION}" )

if ( CMAKE_BUILD_TYPE MATCHES Release )
   message( STATUS "Build type: Release" )
else ( )
   message( STATUS "Build type: Debug" )
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
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Debug>>:${CLANG_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Release>>:${CLANG_RELEASE_FLAGS}>
)
       
target_include_directories( EML
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries( EML
   PUBLIC
      ELL
)

target_sources( EML
   PRIVATE
      "source/EML/allocator_interface.cpp"
      "source/EML/free_list_allocator.cpp"
      "source/EML/monotonic_allocator.cpp"   
      "source/EML/pool_allocator.cpp"
      "source/EML/stack_allocator.cpp"
)

if ( EML_BUILD_TESTS )
   add_custom_command( 
      TARGET EML
      POST_BUILD
      COMMAND ctest --output-on-failure
   )
endif ( )
