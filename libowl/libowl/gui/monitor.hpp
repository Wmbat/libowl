/**
 * @file libowl/gui/monitor.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date 22nd of January 2022
 * @brief
 * @copyright Copyright (C) 2022 wmbat
 */

#ifndef LIBOWL_GUI_MONITOR_HPP_
#define LIBOWL_GUI_MONITOR_HPP_

#include <libowl/detail/x11/connection.hpp>

#include <libmannele/dimension.hpp>
#include <libmannele/position.hpp>

#include <fmt/core.h>

#include <string>

namespace owl::inline v0
{
   /**
    * @brief Struct used to represent rectangles in monitor related contexts. The data is presented
    * in pixels
    */
   struct monitor_dimensions
   {
      i16 x;      ///< The x position of a monitor
      i16 y;      ///< The y position of a monitor
      u16 width;  ///< The width of a monitor
      u16 height; ///< The height of a monitor
   };

   /**
    * @brief Holds the information to identify monitors plugged into the computer
    */
   struct monitor
   {
      std::string name; ///< The name of the monitor

      monitor_dimensions dimensions; ///< The full dimensions of the monitor
   };

#if defined(LIBOWL_USE_X11)
   /**
    * @brief Finds all monitors currently accessible by the X server.
    *
    * @param[in] conn The connection to the X server
    *
    * @return A list of monitors plugged into the computer
    */
   auto list_available_monitors(x11::connection const& conn) -> std::vector<monitor>;
#endif // defined (LIBOWL_USE_X11)
} // namespace owl::inline v0

template <>
struct fmt::formatter<owl::monitor_dimensions>
{
   constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin())
   {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(owl::monitor_dimensions const& dimensions, FormatContext& ctx) -> decltype(ctx.out())
   {
      return format_to(ctx.out(), "{{.x = {}, .y = {}, .width = {}, .height = {}}}", dimensions.x,
                       dimensions.y, dimensions.width, dimensions.height);
   }
};

template <>
struct fmt::formatter<owl::monitor>
{
   constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin())
   {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(owl::monitor const& monitor, FormatContext& ctx) -> decltype(ctx.out())
   {
      return format_to(ctx.out(), "monitor{{.name = {}, .dimensions = {}}}", monitor.name,
                       monitor.dimensions);
   }
};

#endif // LIBOWL_GUI_MONITOR_HPP_
