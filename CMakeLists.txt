cmake_minimum_required( VERSION 3.15 )
project( UVE 
   VERSION 0.0.1 
   DESCRIPTION "A Vulkan based rendering engine"
   LANGUAGES CXX 
)

if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release )
endif( )

option( UVE_BUILD_EXAMPLES "Build examples" ON )

# nlohmann settings
set( JSON_BuildTests OFF CACHE BOOL "" FORCE )

# spirv_cross settings
set( SPIRV_CROSS_SHARED OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_STATIC ON CACHE BOOL "" FORCE )
set( SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_ENABLE_MSL OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "" FORCE )
set( SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake" )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( CLANG_DEBUG_FLAGS "-o0 -g -Wall -Wextra -Weverything -Werror" )
set( CLANG_RELEASE_FLAGS "-o3" )

add_subdirectory( external/volk )
add_subdirectory( external/glslang )
add_subdirectory( external/SPIRV-Cross )
add_subdirectory( external/spdlog )
add_subdirectory( external/nlohmann )
add_subdirectory( external/glm )

add_library( UVE STATIC )

target_compile_features( UVE
   PRIVATE
      cxx_std_17
)

target_compile_options( UVE
   PRIVATE
      $<$<PLATFORM_ID:UNIX>:-pthread>

      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:${CLANG_DEBUG_FLAGS}>

      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:${CLANG_RELEASE_FLAGS}>
)

target_include_directories( UVE
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

if( WIN32 )
   target_compile_definitions( UVE PUBLIC VK_USE_PLATFORM_WIN32_KHR )
elseif( UNIX )
   if( USE_WAYLAND_WSI )
      find_package( Wayland REQUIRED )

      if( NOT WAYLAND_FOUND )
         message( FATAL_ERROR "Wayland development package not found" )
      else( )
         message( STATUS ${WAYLAND_CLIENT_LIBRARIES} )
      endif( )

      target_include_directories( UVE PUBLIC ${WAYLAND_INCLUDE_DIR} )
      target_compile_definitions( UVE PUBLIC VK_USE_PLATFORM_WAYLAND_KHR )
   else( )
      find_package( XCB REQUIRED )

      if( NOT XCB_FOUND )
         message( FATAL_ERROR "XCB development package not found" )
      else( )
         message( STATUS "Using XCB Library" )
      endif( )

      target_compile_definitions( UVE PUBLIC VK_USE_PLATFORM_XCB_KHR )
   endif()
endif( )

set_target_properties( UVE
   PROPERTIES
      CXX_EXTENSIONS OFF
)


target_link_libraries( UVE
   PUBLIC
      ${XCB_LIBRARY}
      vulkan
      volk_headers
      SPIRV
      glslang
      spirv-cross-cpp
      spdlog_header_only
      nlohmann_json
      glm
)

target_sources( UVE
   PRIVATE
      "source/UVE/core/context.cpp"
      "source/UVE/ui/window.cpp"
      "source/UVE/utils/logger.cpp"
      "source/UVE/vk/defines.cpp"
)

if ( UVE_BUILD_EXAMPLES )
   add_subdirectory( examples/triangle_example )
endif ( UVE_BUILD_EXAMPLES )
