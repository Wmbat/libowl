# CMake project initialization

cmake_minimum_required(VERSION 3.14...3.17 FATAL_ERROR)

# Set the project language toolchain, version and description

project(utilities
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
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if(VERMILLON_ENFORCE_CONTRACTS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UTIL_ENFORCE_CONTRACTS) 
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_compile_options(${PROJECT_NAME} PRIVATE
   $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-O0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer>
   $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:-O3>

   $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-O0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer -Wconversion>
   $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:-O3>)

target_link_libraries(${PROJECT_NAME} PUBLIC
    monads::monads
    libcaramel::libcaramel
    range-v3::range-v3
    spdlog::spdlog)

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
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>)

target_sources(${PROJECT_NAME}
    PRIVATE
        utility/logger.cpp)
