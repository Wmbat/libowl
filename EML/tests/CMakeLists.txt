cmake_minimum_required( VERSION 3.15 )

set( CLANG_DEBUG_FLAGS "-o0 -g -Wall -Wextra -Weverything -Werror" )
set( CLANG_RELEASE_FLAGS "-o3" )

add_executable( EML_test )

target_link_libraries( EML_test
   PUBLIC
      EML
      gtest
)

set_target_properties( EML_test
   PROPERTIES
      CXX_EXTENSIONS OFF
)

target_compile_features( EML_test
   PRIVATE
      cxx_std_20
)

target_compile_options( EML_test
   PRIVATE
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:${CLANG_DEBUG_FLAGS}>
      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:${CLANG_RELEASE_FLAGS}>
)

target_sources( EML_test
   PRIVATE
      "EML/main.cpp"
      "EML/free_list_allocator_test.cpp"
      "EML/monotonic_allocator_test.cpp"
      "EML/pool_allocator_test.cpp"
      "EML/stack_allocator_test.cpp"
)

add_test( NAME EML_test COMMAND EML_test )
