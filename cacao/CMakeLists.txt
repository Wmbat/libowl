#CMake project initialization

cmake_minimum_required(VERSION 3.14...3.17 FATAL_ERROR)

#Set the project language toolchain, version and description

project(cacao
   VERSION 0.0.1
   DESCRIPTION ""
   LANGUAGES CXX)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE FORCE)
endif ()

message(STATUS "[${PROJECT_NAME}] ${PROJECT_VERSION}")

if (CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "[${PROJECT_NAME}] Build type: Release")
else ()
    message(STATUS "[${PROJECT_NAME}] Build type: Debug")
endif ()

include(CMakeLists.txt.in)

add_library(${PROJECT_NAME} STATIC)
add_library(cacao::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_HPP_NO_STRUCT_CONSTRUCTORS)

if(VERMILLON_ENFORCE_CONTRACTS)
   target_compile_definitions(${PROJECT_NAME} PRIVATE CACAO_ENFORCE_CONTRACTS) 
endif()

if (VERMILLON_SHOW_DEBUG_LOGGING)
   target_compile_definitions(${PROJECT_NAME} PRIVATE CACAO_DEBUG_LOGGING)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_compile_options(${PROJECT_NAME} PRIVATE
   $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-O0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer>
   $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:-O3>

   $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-O0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer -Wconversion>
   $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:-O3>)

target_link_libraries(${PROJECT_NAME} PUBLIC
    utilities::utilities
    Vulkan-Headers
    glfw
    glslang
    SPIRV
    spirv-cross-core
    glm::glm)

if (VERMILLON_SANITIZE_ADDRESS)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
        target_link_libraries(${PROJECT_NAME} PRIVATE -fsanitize=address)
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
        target_link_libraries(${PROJECT_NAME} PRIVATE -fsanitize=address)
    endif()
endif()

target_include_directories(${PROJECT_NAME}
    SYSTEM INTERFACE
        Vulkan-Headers
        glm::glm
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source)

target_sources(${PROJECT_NAME}
    PRIVATE
        source/cacao/device.cpp
        source/cacao/context.cpp
        source/cacao/gfx/image.cpp
        source/cacao/gfx/index_buffer.cpp
        source/cacao/gfx/vertex_buffer.cpp
        source/cacao/ui/window.cpp
        source/cacao/util/error.cpp
        source/cacao/vulkan/buffer.cpp
        source/cacao/vulkan/command_pool.cpp
        source/cacao/vulkan/descriptor_pool.cpp
        source/cacao/vulkan/descriptor_set_layout.cpp
        source/cacao/vulkan/shader.cpp
        source/cacao/vulkan/swapchain.cpp)
