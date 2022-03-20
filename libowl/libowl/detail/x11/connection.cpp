#include <libowl/detail/x11/connection.hpp>

#include <xcb/xcb.h>

namespace owl::inline v0
{
   namespace
   {
      using unique_atom_reply = std::unique_ptr<xcb_intern_atom_reply_t, void (*)(void*)>;
      using kbd_mapping_reply_ptr =
         std::unique_ptr<xcb_get_keyboard_mapping_reply_t, void (*)(void*)>;

      auto get_protocols_atom_reply(xcb_connection_t* p_conn) -> unique_atom_reply
      {
         using namespace std::literals;

         const auto name = "WM_PROTOCOLS"sv;
         auto cookie = xcb_intern_atom(p_conn, 0, std::size(name), std::data(name));
         return {xcb_intern_atom_reply(p_conn, cookie, nullptr), free};
      }
      auto get_delete_atom_reply(xcb_connection_t* p_conn) -> unique_atom_reply
      {
         using namespace std::literals;

         const auto name = "WM_DELETE_WINDOW"sv;
         auto cookie = xcb_intern_atom(p_conn, 0, std::size(name), std::data(name));
         return {xcb_intern_atom_reply(p_conn, cookie, nullptr), free};
      }

      auto get_keyboard_mapping(xcb_connection_t* p_conn, const xcb_setup_t* p_setup)
         -> kbd_mapping_reply_ptr
      {
         const auto cookie = xcb_get_keyboard_mapping(
            p_conn, p_setup->min_keycode, p_setup->max_keycode - p_setup->min_keycode + 1);

         return {xcb_get_keyboard_mapping_reply(p_conn, cookie, nullptr), free};
      }
   } // namespace

   namespace x11
   {
      auto connect_to_server(spdlog::logger& logger)
         -> tl::expected<connection, server_connection_error_code>
      {
         auto* p_connection = xcb_connect(nullptr, nullptr);
         const i32 error_code = xcb_connection_has_error(p_connection);

         if (error_code == 0)
         {
            logger.debug("connected to X server");

            const xcb_setup_t* p_setup = xcb_get_setup(p_connection);
            const auto kbd_mapping = get_keyboard_mapping(p_connection, p_setup);

            const i32 keysym_count = xcb_get_keyboard_mapping_keysyms_length(kbd_mapping.get());
            const xcb_keysym_t* p_keysyms = xcb_get_keyboard_mapping_keysyms(kbd_mapping.get());

            const auto protocol_reply = get_protocols_atom_reply(p_connection);
            const auto delete_reply = get_delete_atom_reply(p_connection);

            return connection{
               .x_server = unique_x_connection(p_connection, xcb_disconnect),
               .protocol_prop = {.atom = protocol_reply->atom, .delete_atom = delete_reply->atom},
               .min_keycode = p_setup->min_keycode,
               .max_keycode = p_setup->max_keycode,
               .keysyms_per_keycode = kbd_mapping->keysyms_per_keycode,
               .keysyms = {p_keysyms, p_keysyms + keysym_count}}; // NOLINT
         }
         else
         {
            return tl::unexpected(static_cast<server_connection_error_code>(error_code));
         }
      }
   } // namespace x11
} // namespace owl::inline v0
