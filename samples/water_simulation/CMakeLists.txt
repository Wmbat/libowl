#CMake project initialization

cmake_minimum_required( VERSION 3.14...3.17 FATAL_ERROR )

project(water_simulation
    VERSION 0.1.0
    DESCRIPTION "A water simulation system made using the vermillon engine"
    LANGUAGES CXX
)

option(WATER_SIM_SANITIZE_ADDRESS "" OFF)

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

message(STATUS "[${PROJECT_NAME}] Building with address sanitizer: ${WATER_SIM_SANITIZE_ADDRESS}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(CMakeLists.txt.in)

add_executable(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        $<$<PLATFORM_ID:UNIX>:-pthread>

        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer>
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:-o2 -g -fno-omit-frame-pointer>

        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer -Wconversion>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:-o3>
)

if (WATER_SIM_SANITIZE_ADDRESS)
    target_compile_options(${PROJECT_NAME}
        PRIVATE
            $<$<CXX_COMPILER_ID:Clang>>:-sanitize=address>
            $<$<CXX_COMPILER_ID:GNU>>:-sanitize=address>)
endif()

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        vermillon::util
        vermillon::ui
        vermillon::gfx

        EnTT::EnTT 
        tinyobjloader
        tbb
)

if (WATER_SIM_SANITIZE_ADDRESS)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            $<$<CXX_COMPILER_ID:Clang>>:-lasan>
            $<$<CXX_COMPILER_ID:GNU>>:-lasan>)
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_sources(${PROJECT_NAME}
    PRIVATE
        source/water_simulation/main.cpp
        source/water_simulation/simulation.cpp
        source/water_simulation/physics/kernel.cpp
        source/water_simulation/render/camera.cpp
        source/water_simulation/render/depth_buffer.cpp
        source/water_simulation/render/framebuffer.cpp
        source/water_simulation/render/pipeline.cpp
        source/water_simulation/render/pipeline_registry.cpp
        source/water_simulation/render/render_pass.cpp
        source/water_simulation/render/render_system.cpp
        source/water_simulation/render/shader_registry.cpp
)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/resources  # Origin
    ${CMAKE_BINARY_DIR}/samples/water_simulation/resources  # Destination
)
