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

#include <EGL/vk/context.hpp>
#include <alloca.h>

namespace EGL
{
   context::context( std::string_view app_name_in ) :
      app_name( app_name_in ), main_allocator( 1, 5_KB, 10 ), instance_extensions( &main_allocator )
   {
      if ( !IS_VOLK_INIT )
      {
         if ( volkInitialize( ) != VK_SUCCESS )
         {
            throw;
         }
         else
         {
            IS_VOLK_INIT = true;
         }
      }

      std::uint32_t api_version = volkGetInstanceVersion( );

      VkApplicationInfo app_info = { };
      app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      app_info.pNext = nullptr;
      app_info.pApplicationName = app_name.c_str( );
      app_info.applicationVersion = VK_MAKE_VERSION( 0, 0, 0 );
      app_info.pEngineName = "EGL";
      app_info.engineVersion = VK_MAKE_VERSION( 0, 0, 1 );
      app_info.apiVersion = api_version;

      instance_extensions = get_instance_extensions( );

      VkInstanceCreateInfo instance_create_info = { };
      instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      instance_create_info.pNext = nullptr;
      instance_create_info.flags = { };
      instance_create_info.pApplicationInfo = &app_info;
      instance_create_info.enabledLayerCount = 0;
      instance_create_info.ppEnabledLayerNames = nullptr;
      instance_create_info.enabledExtensionCount = instance_extensions.size( );
      instance_create_info.ppEnabledExtensionNames = instance_extensions.data( );

      if ( vkCreateInstance( &instance_create_info, nullptr, &instance ) != VK_SUCCESS )
      {
         // handle error
      }

      volkLoadInstance( instance );
   }
   context::context( context&& other ) : { *this = std::move( other ); }
   context::~context( ) {}

   context& context::operator=( context&& rhs ) {}

   ESL::vector<char const*, ESL::multipool_allocator> context::get_instance_extensions( )
   {
      ESL::vector<char const*, ESL::multipool_allocator> ext( &main_allocator );

      std::uint32_t count = 0;
      vkEnumerateInstanceExtensionProperties( nullptr, &count, nullptr );
      auto* p_ext_prop = reinterpret_cast<VkExtensionProperties*>( alloca( sizeof( VkExtensionProperties ) * count ) );
      vkEnumerateInstanceExtensionProperties( nullptr, &count, p_ext_prop );

      // TODO: don't enable everything by default.

      ext.reserve( count );
      for ( std::size_t i = 0; i < count; ++i )
      {
         ext.emplace_back( p_ext_prop[i].extensionName );
      }

      return ext;
   }

} // namespace EGL
