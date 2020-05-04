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

#include <epona_library/allocators/multipool_allocator.hpp>
#include <epona_library/allocators/pool_allocator.hpp>
#include <epona_library/containers/maps/avl_map.hpp>

#include <gtest/gtest.h>

struct avl_map_test : public testing::Test
{
   using default_map = ESL::avl_map<int, int, ESL::pool_allocator>;

   avl_map_test( ) { main_pool_alloc = ESL::pool_allocator{ { .pool_count = 50, .pool_size = 256 } }; }

   ESL::pool_allocator main_pool_alloc;
};

struct movable
{
   movable( ) = default;
   explicit movable( int i ) : i( i ) {}
   movable( movable const& other ) = delete;
   movable( movable&& other ) { i = std::move( other.i ); }

   movable& operator=( movable const& other ) = delete;
   movable& operator=( movable&& other )
   {
      i = std::move( other.i );
      return *this;
   }

   int i = 0;
};

TEST_F( avl_map_test, default_ctor )
{
   ESL::avl_map<int, int, ESL::pool_allocator> my_map{ &main_pool_alloc };

   EXPECT_EQ( &main_pool_alloc, my_map.get_allocator( ) );
   EXPECT_EQ( 0, my_map.size( ) );
}

TEST_F( avl_map_test, insert_const_ref )
{
   using pair = std::pair<int, int>;

   pair test = { 10, 10 };
   pair test_2 = { 20, 10 };

   default_map my_map{ &main_pool_alloc };

   EXPECT_EQ( &main_pool_alloc, my_map.get_allocator( ) );
   EXPECT_EQ( 0, my_map.size( ) );

   {
      auto [it, took_place] = my_map.insert( test );

      EXPECT_EQ( 1, my_map.size( ) );
      EXPECT_EQ( true, took_place );
      EXPECT_EQ( test.first, it->first );
      EXPECT_EQ( test.second, it->second );
   }
   {
      auto [it, took_place] = my_map.insert( test );

      EXPECT_EQ( 1, my_map.size( ) );
      EXPECT_EQ( false, took_place );
      EXPECT_EQ( test.first, it->first );
      EXPECT_EQ( test.second, it->second );
   }
   {
      auto [it, took_place] = my_map.insert( test_2 );

      EXPECT_EQ( 2, my_map.size( ) );
      EXPECT_EQ( true, took_place );
      EXPECT_EQ( test_2.first, it->first );
      EXPECT_EQ( test_2.second, it->second );
   }
}

TEST_F( avl_map_test, insert_rvalue_ref )
{
   {
      default_map my_map{ &main_pool_alloc };

      EXPECT_EQ( &main_pool_alloc, my_map.get_allocator( ) );
      EXPECT_EQ( 0, my_map.size( ) );

      {
         auto [it, took_place] = my_map.insert( { 10, 10 } );

         EXPECT_EQ( 1, my_map.size( ) );
         EXPECT_EQ( true, took_place );
         EXPECT_EQ( 10, it->first );
         EXPECT_EQ( 10, it->second );
      }
      {
         auto [it, took_place] = my_map.insert( { 10, 20 } );

         EXPECT_EQ( 1, my_map.size( ) );
         EXPECT_EQ( false, took_place );
         EXPECT_EQ( 10, it->first );
         EXPECT_EQ( 10, it->second );
      }
      {
         auto [it, took_place] = my_map.insert( { 20, 20 } );

         EXPECT_EQ( 2, my_map.size( ) );
         EXPECT_EQ( true, took_place );
         EXPECT_EQ( 20, it->first );
         EXPECT_EQ( 20, it->second );
      }
      {
         auto [it, took_place] = my_map.insert( { 30, 30 } );

         EXPECT_EQ( 3, my_map.size( ) );
         EXPECT_EQ( true, took_place );
         EXPECT_EQ( 30, it->first );
         EXPECT_EQ( 30, it->second );
      }

      int i = 1;
      for ( auto& p : my_map )
      {
         EXPECT_EQ( i * 10, p.first );
         EXPECT_EQ( i * 10, p.second );

         ++i;
      }
   }

   default_map my_map{ &main_pool_alloc };

   EXPECT_EQ( &main_pool_alloc, my_map.get_allocator( ) );
   EXPECT_EQ( 0, my_map.size( ) );

   for ( std::size_t i = 0; i < 50; ++i )
   {
      auto [it, took_place] = my_map.insert( { i, i } );

      EXPECT_EQ( i + 1, my_map.size( ) );
      EXPECT_EQ( true, took_place );
      EXPECT_EQ( i, it->first );
      EXPECT_EQ( i, it->second );
   }

   EXPECT_EQ( 50, my_map.size( ) );
   EXPECT_THROW( my_map.insert( { 100, 100 } ), std::bad_alloc );

   for ( int i = 0; auto& it : my_map )
   {
      EXPECT_EQ( i, it.first );
      EXPECT_EQ( i, it.second );

      ++i;
   }
}

TEST_F( avl_map_test, insert_iterator_range )
{
   {
      using pair = std::pair<int, int>;
      using map = ESL::avl_map<int, int, ESL::pool_allocator>;

      map my_map{ &main_pool_alloc };

      my_map.insert( { 20, 20 } );
      my_map.insert( { 10, 10 } );
      my_map.insert( { 30, 30 } );
   }
}
