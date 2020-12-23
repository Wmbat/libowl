#CMake project initialization

cmake_minimum_required(VERSION 3.14...3.17 FATAL_ERROR)

#Set the project language toolchain, version and description

project(vermillon
   VERSION 0.0.1
   DESCRIPTION ""
   LANGUAGES CXX
)

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
add_library(vermillon::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_HPP_NO_STRUCT_CONSTRUCTORS)

if(VERMILLON_ENFORCE_CONTRACTS)
   target_compile_definitions(${PROJECT_NAME} PRIVATE VML_ENFORCE_CONTRACTS) 
endif()

if (VERMILLON_SHOW_DEBUG_LOGGING)
   target_compile_definitions(${PROJECT_NAME} PRIVATE VML_DEBUG_LOGGING)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_compile_options(${PROJECT_NAME} PRIVATE
   $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer>
   $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:-o3>

   $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer -Wconversion>
   $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:-o3>)

target_link_libraries(${PROJECT_NAME} PUBLIC
    spdlog::spdlog
    monads::monads
    libcaramel::libcaramel
    range-v3::range-v3
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
    SYSTEM 
        INTERFACE
            Vulkan-Headers
            glm::glm 
        PUBLIC
            glslang
            SPIRV
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source)

target_sources(${PROJECT_NAME}
    PRIVATE
        source/vermillon/gfx/index_buffer.cpp
        source/vermillon/gfx/vertex_buffer.cpp
        source/vermillon/ui/window.cpp
        source/vermillon/util/logger.cpp
        source/vermillon/util/error.cpp
        source/vermillon/vulkan/sync/fence.cpp
        source/vermillon/vulkan/sync/semaphore.cpp
        source/vermillon/vulkan/buffer.cpp
        source/vermillon/vulkan/command_pool.cpp
        source/vermillon/vulkan/context.cpp
        source/vermillon/vulkan/descriptor_pool.cpp
        source/vermillon/vulkan/descriptor_set_layout.cpp
        source/vermillon/vulkan/device.cpp
        source/vermillon/vulkan/shader.cpp
        source/vermillon/vulkan/swapchain.cpp)
