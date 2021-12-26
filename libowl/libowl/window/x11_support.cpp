#include <libowl/window/x11_support.hpp>

#include <libowl/types.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/remove.hpp>
#include <range/v3/view/transform.hpp>

#include <xcb/randr.h>
#include <xcb/xcb.h>

#include <span>

using reglisse::err;
using reglisse::ok;
using reglisse::result;

namespace rv = ranges::views;

namespace owl::inline v0
{
   namespace x11
   {
      auto connect_to_server(mannele::log_ptr logger)
         -> reglisse::result<unique_connection, server_connection_code>
      {
         auto* p_connection = xcb_connect(nullptr, nullptr);
         const i32 error_code = xcb_connection_has_error(p_connection);

         if (error_code == 0)
         {
            logger.debug("connected to X server");

            return ok(unique_connection(p_connection, xcb_disconnect));
         }
         else
         {
            return err(static_cast<server_connection_code>(error_code));
         }
      }

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

      auto request_resources(const unique_connection& connection, xcb_window_t window)
         -> screen_resources_reply;

      auto list_resource_outputs(const screen_resources_reply& reply)
         -> std::span<const xcb_randr_output_t>;
      auto query_resources_output(xcb_connection_t* p_connection, xcb_randr_output_t output)
         -> output_info_reply;

      auto find_resources_output_info_name(xcb_randr_get_output_info_reply_t* info)
         -> std::string_view;
      auto query_resources_output_crtc(xcb_connection_t* p_connection,
                                       xcb_randr_get_output_info_reply_t* info) -> crtc_info_reply;

      auto gather_output_info(xcb_connection_t* p_connection, output_info_reply&& info)
         -> output_info;

      auto has_monitor(const output_info& info) -> bool;
      auto to_monitor(output_info&& info) -> monitor;

      auto list_available_monitors(const unique_connection& connection) -> std::vector<monitor>
      {
         const xcb_setup_t* p_setup = xcb_get_setup(connection.get());
         xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(p_setup);
         xcb_window_t root_win = screen_iter.data->root;

         if (const auto resources = request_resources(connection, root_win))
         {
            return list_resource_outputs(resources) | rv::transform([&](xcb_randr_output_t output) {
                      return query_resources_output(connection.get(), output);
                   })
                   | rv::transform([&](output_info_reply&& ptr) {
                        return gather_output_info(connection.get(), std::move(ptr));
                     })
                   | rv::remove_if(has_monitor) | rv::transform(to_monitor) | ranges::to_vector;
         }

         return {};
      }

      auto poll_for_event(const unique_connection& connection) -> unique_event
      {
         return {xcb_poll_for_event(connection.get()), free};
      }

      auto request_resources(const unique_connection& connection, xcb_window_t window)
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

         return std::string_view(reinterpret_cast<const char*>(p_name), count); // NOLINT
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
                 .offset = {info.crtc->x, info.crtc->y},
                 .size = {info.crtc->width, info.crtc->height}};
      }
   } // namespace x11
} // namespace owl::inline v0
