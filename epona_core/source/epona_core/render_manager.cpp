#include <epona_core/render_manager.hpp>
#include <epona_library/utils/logger.hpp>

namespace EGL
{
   render_manager::render_manager( ESL::logger* p_logger ) :
      p_logger( p_logger ), main_allocator( { .pool_count = 2, .pool_size = 5_KB, .depth = 5 } ),
      context( &main_allocator )
   {
      if ( !IS_GRAPHIC_ENV_SETUP )
      {
         if ( volkInitialize( ) != VK_SUCCESS )
         {
            throw;
         }
         else if ( !glfwInit( ) )
         {
            throw;
         }
         else
         {
            LOG_INFO( p_logger, "Graphical environment setup" )

            IS_GRAPHIC_ENV_SETUP = true;
         }
      }
   }

   render_manager&& render_manager::set_app_name( std::string_view app_name )
   {
      this->app_name = app_name;
      return std::move( *this );
   }

   render_manager&& render_manager::create_context( )
   {
      context = EGL::vk::runtime( app_name, &main_allocator, p_logger ).create_instance( );
      main_window = EGL::window( app_name, 1080u, 720u );

      LOG_INFO( p_logger, "Basic graphical context setup" );

      return std::move( *this );
   }

   bool render_manager::is_running( ) { return main_window.is_open( ); }

   void render_manager::render( ) { glfwPollEvents( ); }
} // namespace EGL
