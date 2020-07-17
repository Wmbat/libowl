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

cmake_minimum_required( VERSION 3.14...3.17 FATAL_ERROR )

CPMAddPackage(
    NAME gtest
    VERSION 1.10.0
    GITHUB_REPOSITORY google/googletest
    GIT_TAG release-1.10.0)

add_executable(util_test)

set_target_properties(util_test
   PROPERTIES
      CXX_EXTENSIONS OFF)

target_compile_features(util_test
   PRIVATE
      cxx_std_20)

target_compile_options(util_test
   PRIVATE
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-o0 -g>
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:-o3>

      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-o0 -g -fsanitize=address -fsanitize=undefined>
      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:-o3>)

target_link_libraries(util_test
   PUBLIC
      epona::util 
      gtest
   PRIVATE
      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-lasan>
      $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-lubsan>)

target_sources(util_test
   PRIVATE
      util/main.cpp
      util/containers/flat_avl_tree_test.cpp
      util/containers/dynamic_array_test.cpp
      util/detail/monad/either_test.cpp
      util/detail/monad/maybe_test.cpp)

add_test( NAME epona_util_test COMMAND epona_util_test )
