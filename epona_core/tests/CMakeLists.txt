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

cmake_minimum_required( VERSION 3.14 FATAL_ERROR )

set( CLANG_DEBUG_FLAGS -o0 -g )
set( CLANG_RELEASE_FLAGS -o3 )
set( GNU_DEBUG_FLAGS -o0, -g )
set( GNU_RELEASE_FLAGS -o3 )

FetchContent_Declare( gtest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG release-1.10.0
)

FetchContent_MakeAvailable( gtest )

message( STATUS "[${PROJECT_NAME}] Fetching gtest 1.10.0" )

add_executable( epona_core_test )

target_link_libraries( epona_core_test
   PUBLIC
      epona_core
      gtest
)

set_target_properties( epona_core_test
   PROPERTIES
      CXX_EXTENSIONS OFF
)

target_compile_features( epona_core_test
   PRIVATE
      cxx_std_20
)

target_compile_options( epona_core_test
   PRIVATE
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:${CLANG_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:${CLANG_RELEASE_FLAGS}>

      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:${GNU_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:${GNU_RELEASE_FLAGS}>
)

target_precompile_headers( epona_core_test
   PRIVATE
      <gtest/gtest.h>
)

target_sources( epona_core_test
   PRIVATE
      epona_core/main.cpp
      epona_core/containers/flat_map_test.cpp
      epona_core/containers/dynamic_array_test.cpp
)

add_test( NAME epona_core_test COMMAND epona_core_test )
