# CMake project initialization

cmake_minimum_required(VERSION 3.14...3.17 FATAL_ERROR)

# Set the project language toolchain, version and description

project(gfx
   VERSION 0.0.1
   DESCRIPTION "Components related to the graphics on the engine"
   LANGUAGES CXX
)

option(VERMILLON_GFX_STATIC "Build ${PROJECT_NAME} as a static library" ON)
option(VERMILLON_GFX_BUILD_TESTS "Build ${PROJECT_NAME}'s unit tests" OFF)
option(VERMILLON_GFX_BUILD_BENCH "Build ${PROJECT_NAME}'s performance tests" OFF)
option(VERMILLON_GFX_NO_EXCEPTIONS "Build ${PROJECT_NAME} without exceptions" OFF)
option(VERMILLON_GFX_NO_LOGGING "Build ${PROJECT_NAME} without internal logging" OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE FORCE)
endif ()

message(STATUS "[${PROJECT_NAME}] ${PROJECT_VERSION}")

if (CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "[${PROJECT_NAME}] Build type: Release")
else ()
    message(STATUS "[${PROJECT_NAME}] Build type: Debug")
endif ()

message(STATUS "[${PROJECT_NAME}] Building as a static library: ${VERMILLON_GFX_STATIC}")
message(STATUS "[${PROJECT_NAME}] Building unit tests: ${VERMILLON_GFX_BUILD_TESTS}")
message(STATUS "[${PROJECT_NAME}] Building benchmarks: ${VERMILLON_GFX_BUILD_BENCH}")

# Define imported targets

include(CMakeLists.txt.in)

# Define exported targets

add_library(${PROJECT_NAME} STATIC)
add_library(vermillon::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (VERMILLON_GFX_STATIC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VERMILLON_GFX_STATIC) 
endif ()

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_20
)

target_compile_options(${PROJECT_NAME}
   PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer 
        -fsanitize=address>
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:-o3>

        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer
        -Wconversion -fsanitize=address -fsanitize=undefined>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:-o3>
)

target_include_directories(${PROJECT_NAME}
    SYSTEM 
        INTERFACE
            glm::glm 
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        vermillon::util 
        vermillon::ui
        vermillon::vkn
        vermillon::core
        glm::glm
        glfw
    PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-lasan>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-lasan>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-lubsan>
)

target_sources(${PROJECT_NAME}
    PRIVATE
        source/gfx/camera.cpp
        source/gfx/render_manager.cpp
        source/gfx/render_pass.cpp
        source/gfx/memory/camera_buffer.cpp
        source/gfx/memory/index_buffer.cpp
        source/gfx/memory/vertex_buffer.cpp
)
