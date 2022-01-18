#include <libowl/detail/x11/window.hpp>

#include <libowl/system.hpp>

#include <libash/detail/vulkan.hpp>

#include <assert.hpp>

#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xproto.h>

#include <numeric>

namespace owl::inline v0
{
   namespace x11
   {
      window::window(window_create_info&& info) :
         super(*info.p_system, info.name, info.logger), mp_connection(info.conn.x_server.get()),
         m_window_handle(xcb_generate_id(mp_connection)), mp_target_monitor(info.p_target_monitor)
      {
         assert(info.p_target_monitor != nullptr); // NOLINT

         const xcb_setup_t* p_setup = xcb_get_setup(mp_connection);

         xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(p_setup);

         const u32 window_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
         const std::array<u32, 2> window_values = {
            screen_iter.data->black_pixel,
            XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_BUTTON_PRESS
               | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_ENTER_WINDOW
               | XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_FOCUS_CHANGE
               | XCB_EVENT_MASK_EXPOSURE};

         xcb_create_window(mp_connection, XCB_COPY_FROM_PARENT, m_window_handle,
                           screen_iter.data->root, mp_target_monitor->offset.x,
                           mp_target_monitor->offset.y, mp_target_monitor->size.width,
                           mp_target_monitor->size.height, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                           screen_iter.data->root_visual, window_mask,
                           static_cast<const void*>(window_values.data()));

         super::logger().debug("window created on {}", *mp_target_monitor);

         xcb_change_property(mp_connection, XCB_PROP_MODE_REPLACE, m_window_handle,
                             XCB_ATOM_WM_NAME, XCB_ATOM_STRING, sizeof(char8_t) * CHAR_BIT,
                             static_cast<u32>(super::title().size()), super::title().data());
         xcb_change_property(mp_connection, XCB_PROP_MODE_REPLACE, m_window_handle,
                             XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, sizeof(char8_t) * CHAR_BIT,
                             static_cast<u32>(super::title().size()), super::title().data());

         const auto& protocol_prop = info.conn.protocol_prop;
         xcb_change_property(mp_connection, XCB_PROP_MODE_REPLACE, m_window_handle,
                             protocol_prop.atom, XCB_ATOM_ATOM, sizeof(xcb_atom_t) * CHAR_BIT, 1,
                             &(protocol_prop.delete_atom));

         xcb_map_window(mp_connection, m_window_handle);
         xcb_flush(mp_connection);

         const vk::Instance instance = info.instance;

         super::set_surface(
            render_surface(instance.createXcbSurfaceKHRUnique(vk::XcbSurfaceCreateInfoKHR()
                                                                 .setConnection(mp_connection)
                                                                 .setWindow(m_window_handle))));
      }
      window::~window() { xcb_destroy_window(mp_connection, m_window_handle); }

      void window::render(std::chrono::nanoseconds delta_time)
      {
         super::render(delta_time);

         xcb_map_window(mp_connection, m_window_handle);
         xcb_flush(mp_connection);
      }

      auto window::id() const noexcept -> u32 { return m_window_handle; }
   } // namespace x11
} // namespace owl::inline v0
