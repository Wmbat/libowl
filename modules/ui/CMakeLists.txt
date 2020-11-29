# CMake project initialization

cmake_minimum_required(VERSION 3.14...3.17 FATAL_ERROR)

# Set the project language toolchain, version and description

project(ui
   VERSION 0.0.1
   LANGUAGES CXX
)

# User interface declarations

option(VERMILLON_UI_STATIC "Build ${PROJECT_NAME} as a static library" ON)
option(VERMILLON_UI_BUILD_TESTS "Build ${PROJECT_NAME}'s unit tests" OFF)
option(VERMILLON_UI_BUILD_BENCH "Build ${PROJECT_NAME}'s performance tests" OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE FORCE)
endif ()

message(STATUS "[${PROJECT_NAME}] ${PROJECT_VERSION}")

if (CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "[${PROJECT_NAME}] Build type: Release")
else ()
    message(STATUS "[${PROJECT_NAME}] Build type: Debug")
endif ()

message(STATUS "[${PROJECT_NAME}] Building as a static library: ${VERMILLON_UI_STATIC}")

# Define imported targets

include(CMakeLists.txt.in)

# Define exported targets

add_library(${PROJECT_NAME} STATIC)
add_library(vermillon::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (VERMILLON_UI_STATIC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UI_STATIC) 
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        vermillon::util
        vermillon::vkn
        glfw
)

if (VERMILLON_SANITIZE_ADDRESS)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
        target_link_libraries(${PROJECT_NAME} PRIVATE -fsanitize=address)
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
        target_link_libraries(${PROJECT_NAME} PRIVATE -fsanitize=address)
    endif()
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_HPP_NO_STRUCT_CONSTRUCTORS)

target_compile_options(${PROJECT_NAME}
   PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer>
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:-o3>

        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer -Wconversion>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:-o3>)

target_include_directories(${PROJECT_NAME}
    SYSTEM 
        INTERFACE
            Vulkan-Headers
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source)

target_sources(${PROJECT_NAME} PRIVATE source/ui/window.cpp)
