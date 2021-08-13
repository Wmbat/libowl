# CMake project initialization

cmake_minimum_required( VERSION 3.14...3.17 FATAL_ERROR )

project(playground
    VERSION 0.1.0
    DESCRIPTION "playground used for testing"
    LANGUAGES CXX
)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "[${PROJECT_NAME}] Compiling with ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "[${PROJECT_NAME}] ${PROJECT_VERSION}")

if (CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "[${PROJECT_NAME}] Build type: Release")
else ()
    message(STATUS "[${PROJECT_NAME}] Build type: Debug")
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

add_executable(${PROJECT_NAME} playground/source/main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        VERMILLON_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        VERMILLON_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        VERMILLON_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)


target_compile_options(${PROJECT_NAME}
    PRIVATE
        $<$<PLATFORM_ID:UNIX>:-pthread>

        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer
        -fsanitize=address>
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:-o3>

        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer
        -Wconversion -fsanitize=address, -fsanitize=undefined>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:-o3>
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        vermillon::core
        vermillon::gfx
        vermillon::util
        vermillon::vkn

    PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-lasan>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-lasan>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-lubsan>
)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/samples/playground/resources  # Origin
    ${CMAKE_BINARY_DIR}/samples/playground/resources  # Destination
)
