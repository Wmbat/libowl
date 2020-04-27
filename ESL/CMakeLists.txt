# MIT License
#
# Copyright (c) 2020 Wmbat
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required( VERSION 3.12 )
project( ESL
   VERSION 0.2.1
   DESCRIPTION "A standard library"
   LANGUAGES CXX
)

option( ESL_BUILD_TESTS "Build ${PROJECT_NAME}'s unit tests" OFF )
option( ESL_BUILD_BENCH "Build ${PROJECT_NAME}'s performance tests" OFF )
option( ESL_NO_EXCEPTIONS "Build ${PROJECT_NAME} without exceptions" OFF )
option( ESL_NO_LOGGING "Build ${PROJECT_NAME} without internal logging" OFF )

if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release CACHE FORCE )
endif( )

message( STATUS "Build ${PROJECT_NAME}: ${PROJECT_VERSION}" )

if ( CMAKE_BUILD_TYPE MATCHES Release )
   message( STATUS "Build type: Release" )
else ( )
   message( STATUS "Build type: Debug" )
endif ( )

set( CLANG_DEBUG_FLAGS -o0 -Wall -Wextra -Werror -fno-omit-frame-pointer )
set( CLANG_RELEASE_FLAGS -o3 )
set( GNU_DEBUG_FLAGS -o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined )
set( GNU_RELEASE_FLAGS -o3 )

message( STATUS "Build Tests: ${ESL_BUILD_TESTS}" )
message( STATUS "Build Bench: ${ESL_BUILD_TESTS}" )

if ( ESL_BUILD_TESTS ) 
    enable_testing( )

    add_subdirectory( ../external/googletest ${CMAKE_BINARY_DIR}/googletest )
    add_subdirectory( tests )
endif ( )

if ( ESL_BUILD_BENCH )
    add_subdirectory( external/benchmark )
    add_subdirectory( bench )
endif( )

if ( ESL_NO_EXCEPTIONS )
    target_compile_definitions( ${PROJECT_NAME} PRIVATE -DESL_NO_EXCEPTIONS )
endif( )

if ( ESL_NO_LOGGING )
    target_compile_definitions( ${PROJECT_NAME} PRIVATE -DESL_NO_LOGGING )
endif( )

add_subdirectory( external/spdlog )

add_library( ${PROJECT_NAME} STATIC )

set_target_properties( ${PROJECT_NAME}
    PROPERTIES
        CXX_EXTENSIONS OFF
)

target_compile_features( ${PROJECT_NAME}
    PRIVATE
        cxx_std_20
)

target_compile_options( ${PROJECT_NAME}
   PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:${CLANG_DEBUG_FLAGS}>
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:${CLANG_RELEASE_FLAGS}>

        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:${GNU_DEBUG_FLAGS}>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:${GNU_RELEASE_FLAGS}>
)
       
target_include_directories( ${PROJECT_NAME}
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries( ${PROJECT_NAME}
   PUBLIC
      spdlog_header_only

   PRIVATE
      $<$<CXX_COMPILER_ID:GNU>:-lasan>
      $<$<CXX_COMPILER_ID:GNU>:-lubsan>
)

target_sources( ${PROJECT_NAME}
   PRIVATE
      "source/ESL/allocators/monotonic_allocator.cpp"   
      "source/ESL/allocators/multipool_allocator.cpp"
      "source/ESL/allocators/pool_allocator.cpp"
      "source/ESL/allocators/stack_allocator.cpp"
      "source/ESL/utils/logger.cpp"
)
