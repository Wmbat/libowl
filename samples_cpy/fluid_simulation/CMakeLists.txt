#CMake project initialization

cmake_minimum_required( VERSION 3.14...3.17 FATAL_ERROR )

project(fluid_simulation
    VERSION 0.1.0
    DESCRIPTION "An SPH fluid simulation made using the cacao engine"
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

    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-O0 -g -Wall -Wextra -fno-omit-frame-pointer
    -Wconversion>
    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:-O3>

    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-O0 -g -Wall -Wextra -fno-omit-frame-pointer -Wconversion>
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:-O3 -march=native>)

target_link_libraries(${PROJECT_NAME} PUBLIC
    cacao::cacao

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
    source/fluid_simulation/main.cpp
    source/fluid_simulation/simulation.cpp
    source/fluid_simulation/collision/system.cpp
    source/fluid_simulation/physics/kernel.cpp
    source/fluid_simulation/render/camera.cpp
    source/fluid_simulation/render/framebuffer.cpp
    source/fluid_simulation/render/pipeline.cpp
    source/fluid_simulation/render/pipeline_registry.cpp
    source/fluid_simulation/render/render_pass.cpp
    source/fluid_simulation/render/render_system.cpp
    source/fluid_simulation/render/shader_registry.cpp
    source/fluid_simulation/sph/grid.cpp
    source/fluid_simulation/sph/system.cpp)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/resources  # Origin
    ${CMAKE_BINARY_DIR}/samples/fluid_simulation/resources)  # Destination
