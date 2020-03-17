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
   try
   {
      using vector = ESL::vector<copyable, ESL::pool_allocator>;

      copyable test{20};

      auto my_vec = vector( 10, test, &main_pool_alloc );

      EXPECT_EQ( my_vec.get_allocator( ), &main_pool_alloc );
      EXPECT_EQ( my_vec.size( ), 10 );
      EXPECT_EQ( my_vec.capacity( ), 10 );

      for ( auto const& it : my_vec )
      {
         EXPECT_EQ( it.i, 20 );
      }
   }
   catch ( std::bad_alloc& e )
   {
      FAIL( );
   }
}

TEST_F( vector_test, count_ctor )
{
   try
   {
      using vector = ESL::vector<copyable, ESL::pool_allocator>;

      auto my_vec = vector( 10, &main_pool_alloc );

      EXPECT_EQ( my_vec.get_allocator( ), &main_pool_alloc );
      EXPECT_EQ( my_vec.size( ), 10 );
      EXPECT_EQ( my_vec.capacity( ), 10 );
   }
   catch ( std::bad_alloc& e )
   {
      FAIL( );
   }
}

TEST_F( vector_test, iterator_range_ctor )
{
   using vector = ESL::vector<copyable, ESL::pool_allocator>;

   try
   {
      auto my_vec = vector{10, &main_pool_alloc};

      EXPECT_EQ( my_vec.get_allocator( ), &main_pool_alloc );
      EXPECT_EQ( my_vec.size( ), 10 );
      EXPECT_EQ( my_vec.capacity( ), 10 );

      for ( auto& it : my_vec )
      {
         it.i = 20;
         EXPECT_EQ( it.i, 20 );
      }

      auto my_vec2 = vector{my_vec.cbegin( ), my_vec.cend( ), &main_pool_alloc};

      EXPECT_EQ( my_vec2.get_allocator( ), &main_pool_alloc );
      EXPECT_EQ( my_vec2.size( ), 10 );
      EXPECT_EQ( my_vec2.capacity( ), 10 );

      for ( auto const& it : my_vec2 )
      {
         EXPECT_EQ( it.i, 20 );
      }
   }
   catch ( ... )
   {
      FAIL( );
   }
}

TEST_F( vector_test, copy_ctor )
{
   using vector = ESL::vector<copyable, ESL::pool_allocator>;

   copyable test{20};
   try
   {
      auto my_vec = vector{10, test, &main_pool_alloc};

      EXPECT_EQ( my_vec.get_allocator( ), &main_pool_alloc );
      EXPECT_EQ( my_vec.size( ), 10 );
      EXPECT_EQ( my_vec.capacity( ), 10 );

      for ( auto const& it : my_vec )
      {
         EXPECT_EQ( it.i, 20 );
      }

      auto my_vec_2 = vector{my_vec};

      EXPECT_EQ( my_vec_2.get_allocator( ), &main_pool_alloc );
      EXPECT_EQ( my_vec_2.size( ), 10 );
      EXPECT_EQ( my_vec_2.capacity( ), 10 );

      for ( auto const& it : my_vec_2 )
      {
         EXPECT_EQ( it.i, 20 );
      }
   }
   catch ( ... )
   {
      FAIL( );
   }
}

TEST_F( vector_test, copy_new_allocator_ctor )
{
   using vector = ESL::vector<copyable, ESL::pool_allocator>;

   copyable test{20};
   try
   {
      auto my_vec = vector{10, test, &main_pool_alloc};

      EXPECT_EQ( my_vec.get_allocator( ), &main_pool_alloc );
      EXPECT_EQ( my_vec.size( ), 10 );
      EXPECT_EQ( my_vec.capacity( ), 10 );

      for ( auto const& it : my_vec )
      {
         EXPECT_EQ( it.i, 20 );
      }

      auto my_vec2 = vector{my_vec, &backup_pool_alloc};

      EXPECT_EQ( my_vec2.get_allocator( ), &backup_pool_alloc );
      EXPECT_EQ( my_vec2.size( ), 10 );
      EXPECT_EQ( my_vec2.capacity( ), 10 );

      for ( auto const& it : my_vec2 )
      {
         EXPECT_EQ( it.i, 20 );
      }
   }
   catch ( ... )
   {
      FAIL( );
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

TEST_F( vector_test, init_list_ctor )
{
   ESL::vector<int, ESL::pool_allocator> my_vec( {1, 2, 3, 4, 5}, &main_pool_alloc );

   EXPECT_EQ( my_vec.get_allocator( ), &main_pool_alloc );
   EXPECT_EQ( my_vec.size( ), 5 );
   EXPECT_EQ( my_vec.capacity( ), 5 );

   for ( int i = 0; i < my_vec.size( ); ++i )
   {
      EXPECT_EQ( my_vec[i], i + 1 );
   }
}

TEST_F( vector_test, init_list_copy_assign )
{
   ESL::vector<int, ESL::pool_allocator> my_vec{&main_pool_alloc};
   EXPECT_EQ( my_vec.size( ), 0 );
   try
   {
      my_vec = {1, 2, 3};
   }
   catch ( ... )
   {
      FAIL( );
   }

   EXPECT_EQ( my_vec.size( ), 3 );
   EXPECT_EQ( my_vec[0], 1 );
   EXPECT_EQ( my_vec[1], 2 );
   EXPECT_EQ( my_vec[2], 3 );
}

TEST_F( vector_test, assign_count_value )
{
   auto multipool = ESL::multipool_allocator{1, 2048, 2};
   ESL::vector<int, ESL::multipool_allocator> my_vec{2048 / sizeof( int ), 10, &multipool};

   EXPECT_EQ( my_vec.get_allocator( ), &multipool );
   EXPECT_EQ( my_vec.size( ), 2048 / sizeof( int ) );

   std::for_each( my_vec.cbegin( ), my_vec.cend( ), []( int i ) {
      EXPECT_EQ( i, 10 );
   } );

   my_vec.assign( 20, 20 );
   EXPECT_EQ( my_vec.size( ), 20 );

   std::for_each( my_vec.cbegin( ), my_vec.cend( ), []( int i ) {
      EXPECT_EQ( i, 20 );
   } );

   ESL::vector<int, ESL::multipool_allocator> my_vec2{1024 / sizeof( int ), 10, &multipool};
   EXPECT_EQ( my_vec2.get_allocator( ), &multipool );
   EXPECT_EQ( my_vec2.size( ), 1024 / sizeof( int ) );

   std::for_each( my_vec2.cbegin( ), my_vec2.cend( ), []( int i ) {
      EXPECT_EQ( i, 10 );
   } );

   EXPECT_THROW( my_vec2.assign( 2048 / sizeof( int ), 20 ), std::bad_alloc );
}

TEST_F( vector_test, assign_iterator_range ) {}

TEST_F( vector_test, assign_init_list ) {}

TEST_F( vector_test, clear_empty_test )
{
   using vector = ESL::vector<copyable, ESL::pool_allocator>;

   try
   {
      auto my_vec = vector{10, &main_pool_alloc};

      EXPECT_EQ( my_vec.size( ), 10 );
      EXPECT_FALSE( my_vec.empty( ) );

      for ( auto i : my_vec )
      {
         EXPECT_EQ( i.i, 0 );
      }

      my_vec.clear( );

      EXPECT_EQ( my_vec.size( ), 0 );
      EXPECT_TRUE( my_vec.empty( ) );
   }
   catch ( ... )
   {
      FAIL( );
   }
}

TEST_F( vector_test, single_erase_test )
{
   auto my_vec = ESL::vector<copyable, ESL::pool_allocator>{&main_pool_alloc};
   for ( int i = 0; i < 5; ++i )
   {
      my_vec.emplace_back( i + 1 );
   }

   EXPECT_EQ( my_vec.size( ), 5 );

   auto it = my_vec.erase( my_vec.cbegin( ) );
   EXPECT_EQ( it->i, 2 );
   EXPECT_EQ( my_vec[0].i, 2 );
   EXPECT_EQ( my_vec[1].i, 3 );
   EXPECT_EQ( my_vec[2].i, 4 );
   EXPECT_EQ( my_vec[3].i, 5 );
   EXPECT_EQ( my_vec.size( ), 4 );

   my_vec.erase( my_vec.cend( ) - 1 );
   EXPECT_EQ( my_vec[0].i, 2 );
   EXPECT_EQ( my_vec[1].i, 3 );
   EXPECT_EQ( my_vec[2].i, 4 );
   EXPECT_EQ( my_vec.size( ), 3 );
}

TEST_F( vector_test, emplace_back_success_test )
{
   auto my_vec = ESL::vector<copyable, ESL::pool_allocator>( &main_pool_alloc );

   try
   {
      auto const& result = my_vec.emplace_back( 10 );

      EXPECT_EQ( result.i, 10 );
   }
   catch ( ... )
   {
      FAIL( );
   }

   EXPECT_EQ( my_vec[0].i, 10 );

   try
   {
      auto const& result = my_vec.emplace_back( 20 );

      EXPECT_EQ( result.i, 20 );
   }
   catch ( ... )
   {
      FAIL( );
   }

   EXPECT_EQ( my_vec[1].i, 20 );
}
