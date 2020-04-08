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

#include <easy/arbitrary_value.h>
#include <easy/profiler.h>

#include <ESL/allocators/multipool_allocator.hpp>
#include <ESL/allocators/pool_allocator.hpp>
#include <ESL/containers/vector.hpp>

#include <iostream>
#include <vector>

void std_vector_reserve_test( )
{
   EASY_FUNCTION( );

   {
      std::vector<int> my_vec{ };

      EASY_BLOCK( "std_emplace_back_2^8" );

      std::uint8_t it_count_8 = std::numeric_limits<std::uint8_t>::max( );

      my_vec.reserve( it_count_8 );
      for ( int i = 0; i < it_count_8; ++i )
      {
         my_vec.emplace_back( i );
      }

      EASY_END_BLOCK;
   }

   {
      std::vector<int> my_vec{ };

      EASY_BLOCK( "std_emplace_back_2^16" );

      std::uint16_t it_count_16 = std::numeric_limits<std::uint16_t>::max( );

      my_vec.reserve( it_count_16 );
      for ( int i = 0; i < it_count_16; ++i )
      {
         my_vec.emplace_back( i );
      }

      EASY_END_BLOCK;
   }
}

void std_vector_test( )
{
   EASY_FUNCTION( );

   {
      std::vector<int> my_vec{ };

      EASY_BLOCK( "std_emplace_back_2^8" );

      std::uint8_t it_count_8 = std::numeric_limits<std::uint8_t>::max( );

      for ( int i = 0; i < it_count_8; ++i )
      {
         my_vec.emplace_back( i );
      }

      EASY_END_BLOCK;
   }

   {
      std::vector<int> my_vec{ };

      EASY_BLOCK( "std_emplace_back_2^16" );

      std::uint16_t it_count_16 = std::numeric_limits<std::uint16_t>::max( );

      for ( int i = 0; i < it_count_16; ++i )
      {
         my_vec.emplace_back( i );
      }

      EASY_END_BLOCK;
   }
}

void esl_pool_vector_test( ESL::pool_allocator* p_pool_allocator )
{
   EASY_FUNCTION( );

   {
      ESL::vector<int, ESL::pool_allocator> my_vec{ p_pool_allocator };

      EASY_BLOCK( "esl_emplace_back_2^8" );

      std::uint8_t it_count_8 = std::numeric_limits<std::uint8_t>::max( );

      for ( int i = 0; i < it_count_8; ++i )
      {
         my_vec.emplace_back( i );
      }
      EASY_END_BLOCK;
   }

   {
      ESL::vector<int, ESL::pool_allocator> my_vec{ p_pool_allocator };

      EASY_BLOCK( "esl_emplace_back_2^16" );

      std::uint16_t it_count_16 = std::numeric_limits<std::uint16_t>::max( );

      for ( int i = 0; i < it_count_16; ++i )
      {
         my_vec.emplace_back( i );
      }

      EASY_END_BLOCK;
   }
}

int main( )
{
   EASY_PROFILER_ENABLE;

   std::cout << "std_vector_test\n";
   std_vector_test( );

   std::cout << "std_vector_reserve_test\n";
   std_vector_reserve_test( );
   {
      std::cout << "esl_pool_vector_test\n";
      ESL::pool_allocator::create_info const create_info
      { 
         .pool_count = 1,
         .pool_size = sizeof( int ) * std::numeric_limits<std::uint16_t>::max( ) 
      };

      ESL::pool_allocator pool_allocator{ create_info };

      esl_pool_vector_test( &pool_allocator );
   }

   profiler::dumpBlocksToFile( "esl_perf_test.prof" );

   return 0;
}

