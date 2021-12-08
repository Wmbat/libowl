#include <libowl/window/x11_window.hpp>

#include <libash/detail/vulkan.hpp>

#include <assert.hpp>

#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>


namespace owl::inline v0
{
   namespace x11
   {
      window::window(window_create_info&& info) :
         super(info.name, info.logger), mp_connection(info.connection.get()),
         m_window_handle(xcb_generate_id(mp_connection)), mp_target_monitor(info.p_target_monitor)
      {
         assert(info.p_target_monitor != nullptr);

         const xcb_setup_t* p_setup = xcb_get_setup(mp_connection);
         xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(p_setup);

         xcb_create_window(mp_connection, XCB_COPY_FROM_PARENT, m_window_handle,
                           screen_iter.data->root, mp_target_monitor->offset.x,
                           mp_target_monitor->offset.y, mp_target_monitor->size.width,
                           mp_target_monitor->size.height, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                           screen_iter.data->root_visual, 0, nullptr);

         super::logger().debug("window created on {}", *mp_target_monitor);

         xcb_map_window(mp_connection, m_window_handle);
         xcb_change_property(mp_connection, XCB_PROP_MODE_REPLACE, m_window_handle,
                             XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, super::title().size(),
                             super::title().data());
         xcb_change_property(mp_connection, XCB_PROP_MODE_REPLACE, m_window_handle,
                             XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8, super::title().size(),
                             super::title().data());
         xcb_flush(mp_connection);

         const vk::Instance instance = info.instance;

         super::set_surface(
            render_surface(instance.createXcbSurfaceKHRUnique(vk::XcbSurfaceCreateInfoKHR()
                                                                 .setConnection(mp_connection)
                                                                 .setWindow(m_window_handle))));
      }
   } // namespace x11
} // namespace owl::inline v0
