#ifndef LIBOWL_GUI_MONITOR_HPP_
#define LIBOWL_GUI_MONITOR_HPP_

#include <libowl/detail/x11/connection.hpp>

#include <libmannele/dimension.hpp>
#include <libmannele/position.hpp>

#include <fmt/core.h>

#include <string>

namespace owl::inline v0
{
   struct monitor
   {
      std::string name;

      mannele::position_i16 offset;
      mannele::dimension_u16 size;
   };

#if defined(LIBOWL_USE_X11)

   /**
    * @brief Finds all monitors currently accessible by the X server.
    *
    * @param[in] conn
    * @param[in] logger
    *
    * @return
    */
   auto list_available_monitors(const x11::connection& conn) -> std::vector<monitor>;

#endif // defined (LIBOWL_USE_X11)
} // namespace owl::inline v0

template <>
struct fmt::formatter<owl::monitor>
{
   constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin())
   {
      return ctx.begin();
   }

   template <typename FormatContext>
   auto format(const owl::monitor& monitor, FormatContext& ctx) -> decltype(ctx.out())
   {
      return format_to(ctx.out(), "monitor{{ .name = {} .offset = {{{}, {}}} .size = {{{}, {}}} }}",
                       monitor.name, monitor.offset.x, monitor.offset.y, monitor.size.width,
                       monitor.size.height);
   }
};

#endif // LIBOWL_GUI_MONITOR_HPP_
