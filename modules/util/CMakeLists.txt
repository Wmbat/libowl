# CMake project initialization

cmake_minimum_required(VERSION 3.14...3.17 FATAL_ERROR)

# Set the project language toolchain, version and description

project(util
   VERSION 0.0.1
   DESCRIPTION "The general utilities used by the engine"
   LANGUAGES CXX
)

# User interface declarations

option(VERMILLON_UTIL_STATIC "Build ${PROJECT_NAME} as a static library" ON)
option(VERMILLON_UTIL_BUILD_TESTS "Build ${PROJECT_NAME}'s unit tests" OFF)
option(VERMILLON_UTIL_BUILD_BENCH "Build ${PROJECT_NAME}'s performance tests" OFF)
option(VERMILLON_UTIL_SANITIZE_ADDRESS "Build ${PROJECT_NAME} with sanitizers" OFF)
option(VERMILLON_UTIL_NO_EXCEPTIONS "Build ${PROJECT_NAME} without exceptions" OFF)
option(VERMILLON_UTIL_NO_LOGGING "Build ${PROJECT_NAME} without internal logging" OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE FORCE)
endif ()

message(STATUS "[${PROJECT_NAME}] ${PROJECT_VERSION}")

if (CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "[${PROJECT_NAME}] Build type: Release")
else ()
    message(STATUS "[${PROJECT_NAME}] Build type: Debug")
endif ()

message(STATUS "[${PROJECT_NAME}] Building as a static library: ${VERMILLON_UTIL_STATIC}")
message(STATUS "[${PROJECT_NAME}] Building unit tests: ${VERMILLON_UTIL_BUILD_TESTS}")
message(STATUS "[${PROJECT_NAME}] Building benchmarks: ${VERMILLON_UTIL_BUILD_BENCH}")
message(STATUS "[${PROJECT_NAME}] Building without exceptions: ${VERMILLON_UTIL_NO_EXCEPTIONS}")
message(STATUS "[${PROJECT_NAME}] Building without internal logging: ${VERMILLON_UTIL_NO_LOGGING}")
message(STATUS "[${PROJECT_NAME}] Building with address sanitizer: ${VERMILLON_UTIL_SANITIZE_ADDRESS}")

# Define imported targets

include(CMakeLists.txt.in)

if (VERMILLON_UTIL_BUILD_TESTS) 
    enable_testing( )

    add_subdirectory(tests)
endif ()

# Define exported targets

add_library(${PROJECT_NAME} STATIC)
add_library(vermillon::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (VERMILLON_UTIL_STATIC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UTIL_STATIC) 
endif ()

if (VERMILLON_UTIL_NO_EXCEPTIONS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UTIL_NO_EXCEPTIONS)
endif ()

if (VERMILLON_UTIL_NO_LOGGING) 
    target_compile_definitions(${PROJECT_NAME} PRIVATE UTIL_NO_LOGGING)
endif ()

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_EXTENSIONS OFF
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        UTIL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        UTIL_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        UTIL_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_20
)

target_compile_options(${PROJECT_NAME}
   PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer>
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:-o3>

        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer -Wconversion>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:-o3>
)

if (VERMILLON_UTIL_SANITIZE_ADDRESS)
    target_compile_options(${PROJECT_NAME}
        PRIVATE
            $<$<CXX_COMPILER_ID:Clang>>:-sanitize=address>
            $<$<CXX_COMPILER_ID:GNU>>:-sanitize=address>)
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        spdlog::spdlog
        monads::monads
        range-v3::range-v3)

if (VERMILLON_UTIL_SANITIZE_ADDRESS)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            $<$<CXX_COMPILER_ID:Clang>>:-lasan>
            $<$<CXX_COMPILER_ID:GNU>>:-lasan>)
endif()

target_sources(${PROJECT_NAME}
    PRIVATE
        source/util/logger.cpp 
)
