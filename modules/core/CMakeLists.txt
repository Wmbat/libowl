# CMake project initialization

cmake_minimum_required(VERSION 3.14...3.17 FATAL_ERROR)

# Set the project language toolchain, version and description

project(core
   VERSION 0.0.1
   DESCRIPTION "Main components of the epona renderer"
   LANGUAGES CXX
)

# User interface declarations

option(VERMILLON_CORE_STATIC "Build ${PROJECT_NAME} as a static library" ON)
option(VERMILLON_CORE_BUILD_TESTS "Build ${PROJECT_NAME}'s unit tests" OFF)
option(VERMILLON_CORE_BUILD_BENCH "Build ${PROJECT_NAME}'s performance tests" OFF)
option(VERMILLON_CORE_NO_EXCEPTIONS "Build ${PROJECT_NAME} without exceptions" OFF)
option(VERMILLON_CORE_NO_LOGGING "Build ${PROJECT_NAME} without internal logging" OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE FORCE)
endif ()

message(STATUS "[${PROJECT_NAME}] ${PROJECT_VERSION}")

if (CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "[${PROJECT_NAME}] Build type: Release")
else ()
    message(STATUS "[${PROJECT_NAME}] Build type: Debug")
endif ()

message(STATUS "[${PROJECT_NAME}] Building as a static library: ${VERMILLON_CORE_STATIC}")
message(STATUS "[${PROJECT_NAME}] Building unit tests: ${VERMILLON_CORE_BUILD_TESTS}")
message(STATUS "[${PROJECT_NAME}] Building benchmarks: ${VERMILLON_CORE_BUILD_BENCH}")
message(STATUS "[${PROJECT_NAME}] Building without exceptions: ${VERMILLON_CORE_NO_EXCEPTIONS}")
message(STATUS "[${PROJECT_NAME}] Building without internal logging: ${VERMILLON_CORE_NO_LOGGING}")

# Define imported targets

include(CMakeLists.txt.in)

if (VERMILLON_CORE_BUILD_TESTS) 
    enable_testing( )

    #    add_subdirectory(tests)
endif ()

# Define exported targets

add_library(${PROJECT_NAME} STATIC)
add_library(vermillon::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (VERMILLON_CORE_STATIC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VERMILLON_STATIC) 
endif ()

if (VERMILLON_CORE_NO_EXCEPTIONS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ESL_NO_EXCEPTIONS)
endif ()

if (VERMILLON_CORE_NO_LOGGING) 
    target_compile_definitions(${PROJECT_NAME} PRIVATE ESL_NO_LOGGING)
endif ()

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_EXTENSIONS OFF
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        CORE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        CORE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        CORE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

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
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        vermillon::util 
        vermillon::vkn
        glfw
    PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-lasan>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-lasan>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-lubsan>
)

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0") 
    target_precompile_headers( ${PROJECT_NAME} 
        PRIVATE 
            <algorithm> <concepts> <type_traits>
            <cassert> <compare> <cstdint> <cstddef>
            <memory>
    )
endif()

target_sources(${PROJECT_NAME}
    PRIVATE
        source/core/memory/monotonic_allocator.cpp
        source/core/memory/multipool_allocator.cpp
        source/core/memory/pool_allocator.cpp
        source/core/memory/stack_allocator.cpp

        source/core/graphics/gui/widget.cpp

        #        source/core/render_manager.cpp
        source/core/shader_codex.cpp
)
