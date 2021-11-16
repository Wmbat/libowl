#ifndef LIBOWL_WINDOW_X11_WINDOW_HPP_
#define LIBOWL_WINDOW_X11_WINDOW_HPP_

#include <libowl/window.hpp>

namespace owl::inline v0
{
   namespace x11
   {
      struct window_create_info
      {
         std::string_view name;

         const unique_connection& connection;
         const ash::instance& instance;

         monitor* p_target_monitor;

         mannele::log_ptr logger;
      };

      class window : public owl::window
      {
         using super = owl::window;

      public:
         window(window_create_info&& info);

      private:
         xcb_connection_t* mp_connection;
         xcb_window_t m_window_handle;

         monitor* mp_target_monitor;
      };
   } // namespace x11
} // namespace owl::inline v0

#endif // LIBOWL_WINDOW_X11_WINDOW_HPP_
