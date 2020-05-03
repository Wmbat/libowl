#pragma once

#include <EGL/gui/window.hpp>
#include <EGL/vk/runtime.hpp>
#include <ESL/utils/logger.hpp>

namespace EGL
{
   class render_manager
   {
   public:
      render_manager( ESL::logger* p_logger );

      render_manager&& set_app_name( std::string_view app_name );
      render_manager&& create_context( );

      bool is_running( );

      void render( );

   private:
      ESL::logger* p_logger{ nullptr };

      ESL::multipool_allocator main_allocator;

      std::string app_name{ "EGL default app" };

      vk::runtime context;
      window main_window;

      inline static bool IS_GRAPHIC_ENV_SETUP = false;
   };
} // namespace EGL
