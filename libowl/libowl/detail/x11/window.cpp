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
   namespace
   {
      struct desired_event_masks
      {
         u32 mask_category;
         u32 values;
      };

      template <u64 S>
      struct event_masks
      {
         u32 masks;
         std::array<u32, S> values;
      };

      template <std::same_as<desired_event_masks>... T>
      consteval auto set_xcb_window_event_masks(T... masks)
         -> event_masks<std::tuple_size_v<std::tuple<T...>>>
      {
         return {.masks = (masks.mask_category | ...), .values = {masks.values...}};
      }

   } // namespace

   namespace x11
   {
      window::window(window_create_info&& info) :
         super(*info.p_system, info.name, info.logger), mp_connection(info.conn.x_server.get()),
         m_window_handle(xcb_generate_id(mp_connection)), mp_target_monitor(info.p_target_monitor)
      {
         assert(info.p_target_monitor != nullptr); // NOLINT

         const xcb_setup_t* p_setup = xcb_get_setup(mp_connection);

         xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(p_setup);

         constexpr auto window_masks = set_xcb_window_event_masks(desired_event_masks{
            .mask_category = XCB_CW_EVENT_MASK,
            .values = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE
                      | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
                      | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW
                      | XCB_EVENT_MASK_EXPOSURE});

         xcb_create_window(mp_connection, XCB_COPY_FROM_PARENT, m_window_handle,
                           screen_iter.data->root, mp_target_monitor->offset.x,
                           mp_target_monitor->offset.y, mp_target_monitor->size.width,
                           mp_target_monitor->size.height, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                           screen_iter.data->root_visual, window_masks.masks,
                           static_cast<const void*>(window_masks.values.data()));

         super::logger().debug("window created on {}", *mp_target_monitor);

         xcb_map_window(mp_connection, m_window_handle);
         xcb_change_property(mp_connection, XCB_PROP_MODE_REPLACE, m_window_handle,
                             XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                             static_cast<u32>(super::title().size()), super::title().data());
         xcb_change_property(mp_connection, XCB_PROP_MODE_REPLACE, m_window_handle,
                             XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8,
                             static_cast<u32>(super::title().size()), super::title().data());
         xcb_flush(mp_connection);

         const vk::Instance instance = info.instance;

         super::set_surface(
            render_surface(instance.createXcbSurfaceKHRUnique(vk::XcbSurfaceCreateInfoKHR()
                                                                 .setConnection(mp_connection)
                                                                 .setWindow(m_window_handle))));
      }

      void window::render(std::chrono::nanoseconds delta_time)
      {
         super::render(delta_time);

         xcb_map_window(mp_connection, m_window_handle);
         xcb_flush(mp_connection);
      }
   } // namespace x11
} // namespace owl::inline v0