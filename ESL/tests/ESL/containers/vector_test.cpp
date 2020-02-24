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

#include <ESL/allocators/pool_allocator.hpp>
#include <ESL/containers/vector.hpp>

#include <gtest/gtest.h>

struct vector_test : public testing::Test
{
   vector_test( ) = default;

   ESL::pool_allocator main_pool_alloc{2, 2048};
   ESL::pool_allocator backup_pool_alloc{2, 2048};
};

struct copyable
{
   copyable( ) = default;
   copyable( int i ) : i( i ) {}

   int i = 0;
};

struct moveable
{
   moveable( ) = default;
   explicit moveable( int i ) : i( i ) {}
   moveable( moveable const& other ) = delete;
   moveable( moveable&& other ) { i = std::move( other.i ); }

   moveable& operator=( moveable const& other ) = delete;
   moveable& operator=( moveable&& other )
   {
      i = std::move( other.i );
      return *this;
   }

   int i = 0;
};

TEST_F( vector_test, default_ctor )
{
   ESL::vector<int, ESL::pool_allocator> my_vec{&main_pool_alloc};

   EXPECT_EQ( my_vec.get_allocator( ), &main_pool_alloc );
   EXPECT_EQ( my_vec.size( ), 0 );
   EXPECT_EQ( my_vec.capacity( ), 0 );
}

TEST_F( vector_test, count_value_ctor )
{
   copyable test{20};

   try
   {
      ESL::vector<copyable, ESL::pool_allocator> my_vec{10, test, &main_pool_alloc};

      EXPECT_EQ( my_vec.get_allocator( ), &main_pool_alloc );
      EXPECT_EQ( my_vec.size( ), 10 );
      EXPECT_EQ( my_vec.capacity( ), 10 );

      for ( auto const& it : my_vec )
      {
         EXPECT_EQ( it.i, 20 );
      }

      decltype( my_vec ) my_vec2{10, test, &main_pool_alloc};
   }
   catch ( std::bad_alloc )
   {
      FAIL( );
   }

   using vector = ESL::vector<copyable, ESL::pool_allocator>;
   EXPECT_THROW( vector( 10, test, &main_pool_alloc ), std::bad_alloc );
}

TEST_F( vector_test, count_ctor )
{
   ESL::vector<int, ESL::pool_allocator> my_vec{10, &main_pool_alloc};

   EXPECT_EQ( my_vec.get_allocator( ), &main_pool_alloc );
   EXPECT_EQ( my_vec.size( ), 10 );
   EXPECT_EQ( my_vec.capacity( ), 10 );
}

TEST_F( vector_test, iterator_range_ctor )
{
   copyable test{20};
   ESL::vector<copyable, ESL::pool_allocator> my_vec{10, test, &main_pool_alloc};

   EXPECT_EQ( my_vec.get_allocator( ), &main_pool_alloc );
   EXPECT_EQ( my_vec.size( ), 10 );
   EXPECT_EQ( my_vec.capacity( ), 10 );

   for ( auto const& it : my_vec )
   {
      EXPECT_EQ( it.i, 20 );
   }

   decltype( my_vec ) my_vec2{my_vec.begin( ), my_vec.end( ), &main_pool_alloc};

   EXPECT_EQ( my_vec2.get_allocator( ), &main_pool_alloc );
   EXPECT_EQ( my_vec2.size( ), 10 );
   EXPECT_EQ( my_vec2.capacity( ), 10 );

   for ( auto const& it : my_vec2 )
   {
      EXPECT_EQ( it.i, 20 );
   }
}

TEST_F( vector_test, copy_ctor )
{
   copyable test{20};
   ESL::vector<copyable, ESL::pool_allocator> my_vec{10, test, &main_pool_alloc};

   EXPECT_EQ( my_vec.get_allocator( ), &main_pool_alloc );
   EXPECT_EQ( my_vec.size( ), 10 );
   EXPECT_EQ( my_vec.capacity( ), 10 );

   for ( auto const& it : my_vec )
   {
      EXPECT_EQ( it.i, 20 );
   }

   decltype( my_vec ) my_vec2( my_vec );

   EXPECT_EQ( my_vec2.get_allocator( ), &main_pool_alloc );
   EXPECT_EQ( my_vec2.size( ), 10 );
   EXPECT_EQ( my_vec2.capacity( ), 10 );

   for ( auto const& it : my_vec2 )
   {
      EXPECT_EQ( it.i, 20 );
   }
}

TEST_F( vector_test, copy_new_allocator_ctor )
{
   copyable test{20};
   ESL::vector<copyable, ESL::pool_allocator> my_vec{10, test, &main_pool_alloc};

   EXPECT_EQ( my_vec.get_allocator( ), &main_pool_alloc );
   EXPECT_EQ( my_vec.size( ), 10 );
   EXPECT_EQ( my_vec.capacity( ), 10 );

   for ( auto const& it : my_vec )
   {
      EXPECT_EQ( it.i, 20 );
   }

   decltype( my_vec ) my_vec2( my_vec, &backup_pool_alloc );

   EXPECT_EQ( my_vec2.get_allocator( ), &backup_pool_alloc );
   EXPECT_EQ( my_vec2.size( ), 10 );
   EXPECT_EQ( my_vec2.capacity( ), 10 );

   for ( auto const& it : my_vec2 )
   {
      EXPECT_EQ( it.i, 20 );
   }
}

TEST_F( vector_test, move_ctor )
{
   ESL::vector<moveable, ESL::pool_allocator> my_vec{10, &main_pool_alloc};

   EXPECT_EQ( my_vec.get_allocator( ), &main_pool_alloc );
   EXPECT_EQ( my_vec.size( ), 10 );
   EXPECT_EQ( my_vec.capacity( ), 10 );

   for ( auto& it : my_vec )
   {
      it = moveable( 10 );
      EXPECT_EQ( it.i, 10 );
   }

   decltype( my_vec ) my_vec2( std::move( my_vec ) );

   EXPECT_EQ( my_vec2.get_allocator( ), &main_pool_alloc );
   EXPECT_EQ( my_vec2.size( ), 10 );
   EXPECT_EQ( my_vec2.capacity( ), 10 );

   for ( auto& it : my_vec2 )
   {
      EXPECT_EQ( it.i, 10 );
   }

   EXPECT_EQ( my_vec.get_allocator( ), nullptr );
   EXPECT_EQ( my_vec.size( ), 0 );
   EXPECT_EQ( my_vec.capacity( ), 0 );

   decltype( my_vec ) my_vec3{decltype( my_vec )( &main_pool_alloc )};

   EXPECT_EQ( my_vec3.get_allocator( ), &main_pool_alloc );
   EXPECT_EQ( my_vec3.size( ), 0 );
   EXPECT_EQ( my_vec3.capacity( ), 0 );

   for ( auto& it : my_vec3 )
   {
      EXPECT_EQ( it.i, 0 );
   }
}
