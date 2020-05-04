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

set( CLANG_DEBUG_FLAGS "-o0 -g -Wall -Wextra -Weverything -Werror -fno-omit-frame-pointer" )
set( CLANG_RELEASE_FLAGS "-o0" )
set( GNU_DEBUG_FLAGS "-o0 -g -Wall -Wextra -Werror -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined" )
set( GNU_RELEASE_FLAGS "-o0" )

FetchContent_Declare( benchmark
    GIT_REPOSITORY https://github.com/google/benchmark
    GIT_TAG v1.5.0
)

FetchContent_MakeAvailable( benchmark )

add_executable( epona_library_benchmarks )

target_link_libraries( epona_library_benchmarks
   PUBLIC
      epona_library
      benchmark

   PRIVATE
      $<$<CXX_COMPILER_ID:GNU>:-lasan>
      $<$<CXX_COMPILER_ID:GNU>:-lubsan>
)

set_target_properties( epona_library_benchmarks
   PROPERTIES
      CXX_EXTENSIONS OFF
)

target_compile_features( epona_library_benchmarks
   PRIVATE
      cxx_std_20
)

target_compile_options( epona_library_benchmarks
   PRIVATE
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:${CLANG_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:${CLANG_RELEASE_FLAGS}>

      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:${GNU_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:${GNU_RELEASE_FLAGS}>
)

target_sources( epona_library_benchmarks
   PRIVATE
      epona_library/main.cpp
)
