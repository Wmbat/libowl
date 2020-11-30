#CMake project initialization

cmake_minimum_required( VERSION 3.14...3.17 FATAL_ERROR )

project(water_simulation
    VERSION 0.1.0
    DESCRIPTION "A water simulation system made using the vermillon engine"
    LANGUAGES CXX)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

message(STATUS "[${PROJECT_NAME}] ${PROJECT_VERSION}")

include(CMakeLists.txt.in)

add_executable(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

if(VERMILLON_ENFORCE_CONTRACTS)
   target_compile_definitions(${PROJECT_NAME} PUBLIC -DVML_ENFORCE_CONTRACTS) 
endif()

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<PLATFORM_ID:UNIX>:-pthread>

    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -fno-omit-frame-pointer
    -Wconversion>
    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:-o2 -g -fno-omit-frame-pointer>

    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer -Wconversion>
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:-o3>)

target_link_libraries(${PROJECT_NAME} PUBLIC
    vermillon::util
    vermillon::ui
    vermillon::gfx
    vermillon::vkn

    EnTT::EnTT 
    tinyobjloader
    tbb)

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
        PUBLIC
            EnTT::EnTT 
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source)

target_sources(${PROJECT_NAME} PRIVATE
    source/water_simulation/main.cpp
    source/water_simulation/simulation.cpp
    source/water_simulation/collision/system.cpp
    source/water_simulation/physics/kernel.cpp
    source/water_simulation/render/camera.cpp
    source/water_simulation/render/depth_buffer.cpp
    source/water_simulation/render/framebuffer.cpp
    source/water_simulation/render/pipeline.cpp
    source/water_simulation/render/pipeline_registry.cpp
    source/water_simulation/render/render_pass.cpp
    source/water_simulation/render/render_system.cpp
    source/water_simulation/render/shader_registry.cpp
    source/water_simulation/sph/grid.cpp
    source/water_simulation/sph/system.cpp)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/resources  # Origin
    ${CMAKE_BINARY_DIR}/samples/water_simulation/resources)  # Destination
