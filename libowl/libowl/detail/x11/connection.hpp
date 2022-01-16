#ifndef LIBOWL_DETAIL_X11_CONNECTION_HPP_
#define LIBOWL_DETAIL_X11_CONNECTION_HPP_

#include <libowl/types.hpp>

#include <libreglisse/result.hpp>

#include <spdlog/logger.h>

#include <memory>
#include <span>

struct xcb_connection_t;

namespace owl::inline v0
{
   namespace x11
   {
      using unique_x_connection = std::unique_ptr<xcb_connection_t, void (*)(xcb_connection_t*)>;

      enum struct server_connection_error_code
      {
         connection_error = 1,
         extension_not_supported = 2,
         insufficient_memory = 3,
         request_length_exceeded = 4,
         parse_error = 5,
      };

      struct connection
      {
         unique_x_connection x_server;

         u32 window_protocol_atom;
         u32 window_delete_atom;

         u8 min_keycode;
         u8 max_keycode;
         u8 keysyms_per_keycode;
         std::span<const u32> keysyms;
      };

      /**
       * @brief
       *
       * @param [in] logger
       *
       * @return
       */
      auto connect_to_server(spdlog::logger& logger)
         -> reglisse::result<connection, server_connection_error_code>;
   } // namespace x11
} // namespace owl::inline v0

#endif // LIBOWL_DETAIL_X11_CONNECTION_HPP_
