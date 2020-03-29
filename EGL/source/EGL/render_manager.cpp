#include "ESL/utils/logger.hpp"
#include <EGL/render_manager.hpp>

namespace EGL
{
   render_manager::render_manager( ESL::logger* p_logger ) : p_logger( p_logger )
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
            LOG_INFO( p_logger, "Graphic environment setup" )

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
      context = EGL::context( app_name, p_logger ).create_instance( );
      main_window = EGL::window( app_name, 1080u, 720u );

      return std::move( *this );
   }

   bool render_manager::is_running( ) { return main_window.is_open( ); }

   void render_manager::render( ) { glfwPollEvents( ); }
} // namespace EGL
