/**
 * MIT License
 *
 * Copyright (c) 2020 Wmbat
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <ESL/allocators/multipool_allocator.hpp>
#include <ESL/allocators/pool_allocator.hpp>
#include <ESL/containers/vector.hpp>

#include <benchmark/benchmark.h>

#include <iostream>
#include <string>

static void std_vector_int( benchmark::State& state )
{
   for ( auto _ : state )
   {
      std::vector<int> v1( state.range( 0 ), state.range( 0 ) );

      benchmark::DoNotOptimize( v1.data( ) );
      benchmark::ClobberMemory( );
   }
}

static void std_vector_int_2( benchmark::State& state )
{
   for ( auto _ : state )
   {
      std::vector<int> v1( state.range( 0 ), state.range( 0 ) );
      std::vector<int> v2( state.range( 0 ), state.range( 0 ) );

      benchmark::DoNotOptimize( v1.data( ) );
      benchmark::DoNotOptimize( v2.data( ) );
      benchmark::ClobberMemory( );
   }
}

static void esl_vector_int( benchmark::State& state )
{
   ESL::pool_allocator allocator(
      { .pool_count = 1, .pool_size = static_cast<size_t>( state.range( 0 ) * sizeof( int ) ) } );

   for ( auto _ : state )
   {
      ESL::vector<int, ESL::pool_allocator> v1( state.range( 0 ), state.range( 0 ), &allocator );

      benchmark::DoNotOptimize( v1.data( ) );
      benchmark::ClobberMemory( );
   }
}

static void esl_vector_int_2( benchmark::State& state )
{
   ESL::pool_allocator allocator(
      { .pool_count = 2, .pool_size = static_cast<size_t>( state.range( 0 ) * sizeof( int ) ) } );

   for ( auto _ : state )
   {
      ESL::vector<int, ESL::pool_allocator> v1( state.range( 0 ), state.range( 0 ), &allocator );
      ESL::vector<int, ESL::pool_allocator> v2( state.range( 0 ), state.range( 0 ), &allocator );

      benchmark::DoNotOptimize( v1.data( ) );
      benchmark::DoNotOptimize( v2.data( ) );
      benchmark::ClobberMemory( );
   }
}

BENCHMARK( std_vector_int )->RangeMultiplier( 2 )->Range( 8, 8 << 15 );
BENCHMARK( esl_vector_int )->RangeMultiplier( 2 )->Range( 8, 8 << 15 );
BENCHMARK( std_vector_int_2 )->RangeMultiplier( 2 )->Range( 8, 8 << 15 );
BENCHMARK( esl_vector_int_2 )->RangeMultiplier( 2 )->Range( 8, 8 << 15 );

BENCHMARK_MAIN( );
