cmake_minimum_required( VERSION 3.14...3.17 FATAL_ERROR )

project( epona_core
   VERSION 0.0.1
   DESCRIPTION "Main components of the epona renderer"
   LANGUAGES CXX
)

option( EPONA_CORE_STATIC "Build ${PROJECT_NAME} as a static library" ON )
option( EPONA_CORE_BUILD_TESTS "Build ${PROJECT_NAME}'s unit tests" OFF )
option( EPONA_CORE_BUILD_BENCH "Build ${PROJECT_NAME}'s performance tests" OFF )
option( EPONA_CORE_NO_EXCEPTIONS "Build ${PROJECT_NAME} without exceptions" OFF )
option( EPONA_CORE_NO_LOGGING "Build ${PROJECT_NAME} without internal logging" OFF )


if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release CACHE FORCE )
endif( )

message( STATUS "[${PROJECT_NAME}] ${PROJECT_VERSION}" )

if ( CMAKE_BUILD_TYPE MATCHES Release )
   message( STATUS "[${PROJECT_NAME}] Build type: Release" )
else ( )
   message( STATUS "[${PROJECT_NAME}] Build type: Debug" )
endif ( )

message( STATUS "[${PROJECT_NAME}] Building as a static library: ${EPONA_CORE_STATIC}" )
message( STATUS "[${PROJECT_NAME}] Building unit tests: ${EPONA_CORE_BUILD_TESTS}" )
message( STATUS "[${PROJECT_NAME}] Building benchmarks: ${EPONA_CORE_BUILD_BENCH}" )
message( STATUS "[${PROJECT_NAME}] Building without exceptions: ${EPONA_CORE_NO_EXCEPTIONS}" )
message( STATUS "[${PROJECT_NAME}] Building without internal logging: ${EPONA_CORE_NO_LOGGING}" )

# spirv_cross settings
set( SPIRV_CROSS_SHARED OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_STATIC ON CACHE BOOL "" FORCE )
set( SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_ENABLE_MSL OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON CACHE BOOL "" FORCE )

# GLFW settings
set( GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE )
set( GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE )

include( CMakeLists.txt.in )

if ( EPONA_CORE_BUILD_TESTS ) 
    enable_testing( )

    add_subdirectory( tests )
endif ( )

add_library( ${PROJECT_NAME} STATIC )

if (EPONA_CORE_STATIC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DEPONA_STATIC) 
endif()

if ( EPONA_CORE_NO_EXCEPTIONS )
    target_compile_definitions( ${PROJECT_NAME} PRIVATE -DESL_NO_EXCEPTIONS )
endif( )

if ( EPONA_CORE_NO_LOGGING )
    target_compile_definitions( ${PROJECT_NAME} PRIVATE -DESL_NO_LOGGING )
endif( )

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
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Weverything -Werror -fno-omit-frame-pointer>
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:-o3>

        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer
        -Wconversion -fsanitize=address -fsanitize=undefined>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:-o3>
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
      spdlog::spdlog
   PRIVATE
      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-lasan>
      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-lubsan>
)

if ( ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0" ) 
    target_precompile_headers( ${PROJECT_NAME} 
        PRIVATE 
            <algorithm> <concepts> <type_traits>
            <cassert> <compare> <cstdint> <cstddef>
            <memory> <spdlog/spdlog.h>

    )
endif()

target_sources( ${PROJECT_NAME}
   PRIVATE
      source/epona_core/gui/widget.cpp
      source/epona_core/gui/window.cpp

      source/epona_core/memory/monotonic_allocator.cpp
      source/epona_core/memory/multipool_allocator.cpp
      source/epona_core/memory/pool_allocator.cpp
      source/epona_core/memory/stack_allocator.cpp

      source/epona_core/details/logger.cpp

      source/epona_core/vk/runtime.cpp
      source/epona_core/vk/device.cpp
      source/epona_core/vk/instance.cpp
      source/epona_core/vk/physical_device.cpp
      source/epona_core/vk/defines.cpp
      source/epona_core/vk/swapchain.cpp

      source/epona_core/render_manager.cpp
)
