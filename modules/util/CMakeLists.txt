# CMake project initialization

cmake_minimum_required(VERSION 3.14...3.17 FATAL_ERROR)

# Set the project language toolchain, version and description

project(util
   VERSION 0.0.1
   DESCRIPTION "The general utilities used by the engine"
   LANGUAGES CXX
)

# User interface declarations

option(MELODIE_UTIL_STATIC "Build ${PROJECT_NAME} as a static library" ON)
option(MELODIE_UTIL_BUILD_TESTS "Build ${PROJECT_NAME}'s unit tests" OFF)
option(MELODIE_UTIL_BUILD_BENCH "Build ${PROJECT_NAME}'s performance tests" OFF)
option(MELODIE_UTIL_NO_EXCEPTIONS "Build ${PROJECT_NAME} without exceptions" OFF)
option(MELODIE_UTIL_NO_LOGGING "Build ${PROJECT_NAME} without internal logging" OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE FORCE)
endif ()

message(STATUS "[${PROJECT_NAME}] ${PROJECT_VERSION}")

if (CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "[${PROJECT_NAME}] Build type: Release")
else ()
    message(STATUS "[${PROJECT_NAME}] Build type: Debug")
endif ()

message(STATUS "[${PROJECT_NAME}] Building as a static library: ${MELODIE_UTIL_STATIC}")
message(STATUS "[${PROJECT_NAME}] Building unit tests: ${MELODIE_UTIL_BUILD_TESTS}")
message(STATUS "[${PROJECT_NAME}] Building benchmarks: ${MELODIE_UTIL_BUILD_BENCH}")
message(STATUS "[${PROJECT_NAME}] Building without exceptions: ${MELODIE_UTIL_NO_EXCEPTIONS}")
message(STATUS "[${PROJECT_NAME}] Building without internal logging: ${MELODIE_UTIL_NO_LOGGING}")

# Define imported targets

include(CMakeLists.txt.in)

if (MELODIE_UTIL_BUILD_TESTS) 
    enable_testing( )

    add_subdirectory(tests)
endif ()

# Define exported targets

add_library(${PROJECT_NAME} STATIC)
add_library(melodie::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (MELODIE_UTIL_STATIC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UTIL_STATIC) 
endif ()

if (MELODIE_UTIL_NO_EXCEPTIONS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UTIL_NO_EXCEPTIONS)
endif ()

if (MELODIE_UTIL_NO_LOGGING) 
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
        spdlog::spdlog
        monads::monads
        mpark_patterns
    PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-lasan>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-lasan>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-lubsan>
)

target_sources(${PROJECT_NAME}
    PRIVATE
        source/util/logger.cpp 
)
