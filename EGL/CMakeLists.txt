cmake_minimum_required( VERSION 3.12 )
project( EGL
   VERSION 0.0.1
   DESCRIPTION "A Vulkan graphics library"
   LANGUAGES CXX
)

if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release CACHE FORCE )
endif( )

message( STATUS "Build EGL: 0.0.1" )

if ( CMAKE_BUILD_TYPE MATCHES Release )
   message( STATUS "Build type: Release" )
else( )
   message( STATUS "Build type: Debug" )
endif( )

# compilation settings
set( CLANG_DEBUG_FLAGS "-o0 -g -Wall -Wextra -Weverything -Werror" )
set( CLANG_RELEASE_FLAGS "-o3" )
set( GNU_DEBUG_FLAGS "-o0, -g -Wall -Wextra -Werror" )
set( GNU_RELEASE_FLAGS "-o3" )

# spirv_cross settings
set( SPIRV_CROSS_SHARED OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_STATIC ON CACHE BOOL "" FORCE )
set( SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_ENABLE_MSL OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON CACHE BOOL "" FORCE )

# external dependencies
add_subdirectory( external/Vulkan-Headers )
add_subdirectory( external/volk )
add_subdirectory( external/glslang )
add_subdirectory( external/SPIRV-Cross )

add_library( EGL STATIC )

set_target_properties( EGL
   PROPERTIES
      CXX_EXTENSIONS OFF
)

target_compile_features( EGL
   PRIVATE
      cxx_std_20
)

target_compile_options( EGL
   PRIVATE
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:${CLANG_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:${CLANG_RELEASE_FLAGS}>

      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:${GNU_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:${GNU_RELEASE_FLAGS}>
)

target_include_directories( EGL 
   PUBLIC 
      $<INSTALL_INTERFACE:include>    
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
   PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries( EGL
   PUBLIC
      Vulkan-Headers
      volk_headers
      glslang
      SPIRV
      ESL
)

target_sources( EGL
   PRIVATE
      "source/EGL/render_manager.cpp"
      "source/EGL/vk/context.cpp"
      "source/EGL/vk/defines.cpp"
)
