cmake_minimum_required( VERSION 3.14...3.17 FATAL_ERROR )

project( epona_core
   VERSION 0.0.1
   DESCRIPTION "Main components of the epona renderer"
   LANGUAGES CXX
)

if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release CACHE FORCE )
endif( )

# compilation settings
set( CLANG_DEBUG_FLAGS "-o0 -g -Wall -Wextra -Weverything -Werror -fno-omit-frame-pointer" )
set( CLANG_RELEASE_FLAGS "-o3" )
set( GNU_DEBUG_FLAGS "-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined" )
set( GNU_RELEASE_FLAGS "-o3" )

message( STATUS "[${PROJECT_NAME}] ${PROJECT_VERSION} - ${CMAKE_BUILD_TYPE}" )

# spirv_cross settings
set( SPIRV_CROSS_SHARED OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_STATIC ON CACHE BOOL "" FORCE )
set( SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_ENABLE_MSL OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON CACHE BOOL "" FORCE )

include( CMakeLists.txt.in )

add_library( ${PROJECT_NAME} STATIC )

set_target_properties( ${PROJECT_NAME}
   PROPERTIES
      CXX_EXTENSIONS OFF
)

target_compile_definitions( ${PROJECT_NAME}
   PRIVATE
      CORE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
      CORE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
      CORE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
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
      Vulkan-Headers
      volk_headers
      glslang
      SPIRV
      glfw
      epona_library
)

target_sources( ${PROJECT_NAME}
   PRIVATE
      source/epona_core/gui/widget.cpp
      source/epona_core/gui/window.cpp
      source/epona_core/vk/runtime.cpp
      source/epona_core/vk/defines.cpp
      source/epona_core/render_manager.cpp
)
