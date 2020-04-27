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

/**
 * @mainpage Test main page
 */

#include <EGL/render_manager.hpp>
#include <ESL/allocators/pool_allocator.hpp>
#include <ESL/utils/logger.hpp>

#include <map>

#include <ESL/containers/vector.hpp>

int main( )
{
   auto pool = ESL::pool_allocator{ { .pool_count = 1, .pool_size = 1024 } };

   ESL::hybrid_vector<int, 4, decltype(pool)> my_vec{&pool};
   my_vec.push_back( 10 );

   auto main_logger = ESL::logger( "main_logger" );

   auto render_manager = EGL::render_manager( &main_logger ).set_app_name( "My App" ).create_context( );

   while ( render_manager.is_running( ) )
   {
      render_manager.render( );
   }

   return 0;
}
