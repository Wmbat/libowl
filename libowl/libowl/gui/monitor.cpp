/**
 * @file libowl/gui/monitor.cpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date 22nd of January 2022
 * @brief
 * @copyright Copyright (C) 2022 wmbat
 */

#include <libowl/gui/monitor.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/remove.hpp>
#include <range/v3/view/transform.hpp>

#include <xcb/randr.h>
#include <xcb/xcb.h>

namespace rv = ranges::views;

namespace owl::inline v0
{
#if defined(LIBOWL_USE_X11)

   namespace
   {
      template <typename Type>
      using xcb_handle = std::unique_ptr<Type, void (*)(void*)>;
      using screen_resources_reply = xcb_handle<xcb_randr_get_screen_resources_reply_t>;
      using crtc_info_reply = xcb_handle<xcb_randr_get_crtc_info_reply_t>;
      using output_info_reply = xcb_handle<xcb_randr_get_output_info_reply_t>;

      struct output_info
      {
         std::string name;
         crtc_info_reply crtc;
      };

      auto request_resources(const x11::unique_x_connection& connection, xcb_window_t window)
         -> screen_resources_reply
      {
         const auto cookie = xcb_randr_get_screen_resources(connection.get(), window);
         auto* p_reply = xcb_randr_get_screen_resources_reply(connection.get(), cookie, nullptr);

         return {p_reply, free};
      }

      auto list_resource_outputs(const screen_resources_reply& reply)
         -> std::span<const xcb_randr_output_t>
      {
         const i32 count = xcb_randr_get_screen_resources_outputs_length(reply.get());
         const auto* p_outputs = xcb_randr_get_screen_resources_outputs(reply.get());

         return {p_outputs, static_cast<u64>(count)};
      }
      auto query_resources_output(xcb_connection_t* p_connection, xcb_randr_output_t output)
         -> output_info_reply
      {
         const auto cookie = xcb_randr_get_output_info_unchecked(p_connection, output, 0);
         auto* p_reply = xcb_randr_get_output_info_reply(p_connection, cookie, nullptr);

         return {p_reply, free};
      }

      auto find_resources_output_info_name(xcb_randr_get_output_info_reply_t* info)
         -> std::string_view
      {
         const i32 count = xcb_randr_get_output_info_name_length(info);
         const u8* p_name = xcb_randr_get_output_info_name(info);

         // NOLINTNEXTLINE
         return std::string_view(reinterpret_cast<const char*>(p_name),
                                 static_cast<std::size_t>(count));
      }
      auto query_resources_output_crtc(xcb_connection_t* p_connection,
                                       xcb_randr_get_output_info_reply_t* info) -> crtc_info_reply
      {
         const auto cookie = xcb_randr_get_crtc_info_unchecked(p_connection, info->crtc, 0);
         auto* p_crtc = xcb_randr_get_crtc_info_reply(p_connection, cookie, nullptr);

         return {p_crtc, free};
      }

      auto gather_output_info(xcb_connection_t* p_connection, output_info_reply&& info)
         -> output_info
      {
         return {.name = std::string(find_resources_output_info_name(info.get())),
                 .crtc = query_resources_output_crtc(p_connection, info.get())};
      }

      auto has_monitor(const output_info& info) -> bool { return info.crtc == nullptr; }

      auto to_monitor(output_info&& info) -> monitor
      {
         return {.name = std::move(info.name),
                 .dimensions = {.x = info.crtc->x,
                                .y = info.crtc->y,
                                .width = info.crtc->width,
                                .height = info.crtc->height}};
      }

   } // namespace

   auto list_available_monitors(const x11::connection& conn) -> std::vector<monitor>
   {
      const xcb_setup_t* p_setup = xcb_get_setup(conn.x_server.get());
      xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(p_setup);
      xcb_window_t root_win = screen_iter.data->root;

      if (const auto resources = request_resources(conn.x_server, root_win))
      {
         return list_resource_outputs(resources) | rv::transform([&](xcb_randr_output_t output) {
                   return query_resources_output(conn.x_server.get(), output);
                })
                | rv::transform([&](output_info_reply&& ptr) {
                     return gather_output_info(conn.x_server.get(), std::move(ptr));
                  })
                | rv::remove_if(has_monitor) | rv::transform(to_monitor) | ranges::to_vector;
      }

      return {};
   }

#endif // defined (LIBOWL_USE_X11)
} // namespace owl::inline v0
