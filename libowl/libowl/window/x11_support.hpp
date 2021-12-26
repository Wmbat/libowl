/**
 * @file libowl/window/x11_support.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @copyright Copyright (C) 2021 wmbat.
 * @brief
 */

#ifndef LIBOWL_WINDOW_X11_SUPPORT_HPP_
#define LIBOWL_WINDOW_X11_SUPPORT_HPP_

#include <libowl/window/monitor.hpp>

#include <libreglisse/result.hpp>

#include <libmannele/logging/log_ptr.hpp>

#include <xcb/xcb.h>

#include <memory>

namespace owl::inline v0
{
   namespace x11
   {
      using unique_connection = std::unique_ptr<xcb_connection_t, void (*)(xcb_connection_t*)>;
      using unique_event = std::unique_ptr<xcb_generic_event_t, void (*)(void*)>;

      enum struct server_connection_code
      {
         error = XCB_CONN_ERROR,
         extension_not_supported = XCB_CONN_CLOSED_EXT_NOTSUPPORTED,
         insufficient_memory = XCB_CONN_CLOSED_MEM_INSUFFICIENT,
         request_lenght_exceeded = XCB_CONN_CLOSED_REQ_LEN_EXCEED,
         parse_error = XCB_CONN_CLOSED_PARSE_ERR,
         invalid_screen = XCB_CONN_CLOSED_PARSE_ERR
      };

      /**
       * @brief Establishes a connection to the X server
       *
       * @param [in] logger
       *
       * @return
       */
      auto connect_to_server(mannele::log_ptr logger)
         -> reglisse::result<unique_connection, server_connection_code>;

      /**
       * @brief Finds all monitors currently accessible by the X server.
       *
       * @param[in] connection
       * @param[in] logger
       *
       * @return
       */
      auto list_available_monitors(const unique_connection& connection) -> std::vector<monitor>;

      /**
       *
       */
      auto poll_for_event(const unique_connection& connection) -> unique_event;
   } // namespace x11
} // namespace owl::inline v0

#endif // LIBOWL_WINDOW_X11_SUPPORT_HPP_
