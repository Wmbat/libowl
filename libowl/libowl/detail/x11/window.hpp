#ifndef LIBOWL_WINDOW_DETAIL_X11_WINDOW_HPP_
#define LIBOWL_WINDOW_DETAIL_X11_WINDOW_HPP_

#include <libowl/window.hpp>

namespace owl::inline v0
{
   namespace x11
   {
      struct window_create_info
      {
         system* p_system;

         std::string_view name;

         const connection& conn;
         const ash::instance& instance;

         monitor* p_target_monitor;

         spdlog::logger& logger;
      };

      class window : public owl::window
      {
         using super = owl::window;

      public:
         window(window_create_info&& info);
         window(const window& wnd) = delete;
         window(window&& wnd) noexcept = delete;
         ~window() override = default;

         auto operator=(const window& wnd) = delete;
         auto operator=(window&& wnd) noexcept = delete;

         void render(std::chrono::nanoseconds delta_time) override;

      private:
         xcb_connection_t* mp_connection;
         xcb_window_t m_window_handle;

         monitor* mp_target_monitor;
      };
   } // namespace x11
} // namespace owl::inline v0

#endif // LIBOWL_WINDOW_DETAIL_X11_WINDOW_HPP_
