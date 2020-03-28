cmake_minimum_required( VERSION 3.12 )
project( EWL
   VERSION 0.0.1
   DESCRIPTION "A Window management library"
   LANGUAGES CXX
)

if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release CACHE FORCE )
endif( )

message( STATUS "Build ${PROJECT_NAME}: ${PROJECT_VERSION}" )

if ( CMAKE_BUILD_TYPE MATCHES Release )
   message( STATUS "Build type: Release" )
else ( )
   message( STATUS "Build type: Debug" )
endif ( )

set( CLANG_DEBUG_FLAGS "-o0 -g -Wall -Wextra -Weverything -Werror -fno-omit-frame-pointer" )
set( CLANG_RELEASE_FLAGS "-o3" )
set( GNU_DEBUG_FLAGS "-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined" )
set( GNU_RELEASE_FLAGS "-o3" )

add_subdirectory( external/glfw )

add_library( ${PROJECT_NAME} STATIC )

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
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:${CLANG_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:${CLANG_RELEASE_FLAGS}>

      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:${GNU_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:${GNU_RELEASE_FLAGS}>
)

target_include_directories( ${PROJECT_NAME}
   PUBLIC
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
   PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries( ${PROJECT_NAME}
   PUBLIC
      glfw
      ESL
      EGL
)

target_sources( ${PROJECT_NAME}
   PRIVATE
      "source/EWL/widget.cpp"
      "source/EWL/window.cpp"
)
