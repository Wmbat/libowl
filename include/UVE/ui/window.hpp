#pragma once

/* INCLUDES */
#include <UVE/core/common_types.hpp>
#include <UVE/ui/event.hpp>
#include <UVE/utils/delegate.hpp>
#include <UVE/utils/message_handler.hpp>
#include <UVE/vk/core.hpp>

#include <glm/vec2.hpp>
#include <spdlog/logger.h>

#if defined( VK_USE_PLATFORM_XCB_KHR )
#   include <xcb/xcb.h>
#endif

#include <memory>
#include <string>
#include <variant>

class logger;

namespace ui
{
   struct window_create_info
   {
      std::string title = "Default Luciole Window";
      glm::uvec2 position = {100, 100};
      glm::uvec2 size = {1080, 720};

      logger* p_logger = nullptr;
   };

   class window
   {
   public:
#if defined( VK_USE_PLATFORM_XCB_KHR )
      using xcb_connection_uptr = std::unique_ptr<xcb_connection_t, delegate<void( xcb_connection_t* )>>;
      using xcb_intern_atom_uptr = std::unique_ptr<xcb_intern_atom_reply_t, delegate<void( xcb_intern_atom_reply_t* )>>;
#endif

   public:
      window( ) = default;
      window( window_create_info const& create_info );
      window( window const& wnd ) = delete;
      window( window&& wnd );
      ~window( );

      window& operator=( window const& rhs ) = delete;
      window& operator=( window&& rhs );

      bool is_open( );

      void poll_events( );

      bool check_WSI_support( VkPhysicalDevice, UVE::uint32 queue_family_index ) const;

      [[nodiscard]] VkSurfaceKHR create_surface( VkInstance instance ) const;

      template <class C>
      std::enable_if_t<std::is_same_v<C, key_event_delg>, void> add_callback( const C& callback )
      {
         key_handler.add_callback( callback );
      }

      template <class C>
      std::enable_if_t<std::is_same_v<C, mouse_button_event_delg>, void> add_callback( const C& callback )
      {
         mouse_button_handler.add_callback( callback );
      }

      template <class C>
      std::enable_if_t<std::is_same_v<C, mouse_motion_event_delg>, void> add_callback( const C& callback )
      {
         mouse_motion_handler.add_callback( callback );
      }

      template <class C>
      std::enable_if_t<std::is_same_v<C, window_close_event_delg>, void> add_callback( const C& callback )
      {
         window_close_handler.add_callback( callback );
      }

      template <class C>
      std::enable_if_t<std::is_same_v<C, framebuffer_resize_event_delg>, void> add_callback( const C& callback )
      {
         framebuffer_resize_handler.add_callback( callback );
      }

      [[nodiscard]] glm::uvec2 get_size( ) const;

   private:
      std::string title;
      glm::uvec2 position;
      glm::uvec2 size;

      logger* p_logger;

      bool is_wnd_open;
      bool is_fullscreen;

#if defined( VK_USE_PLATFORM_XCB_KHR )
      xcb_connection_uptr p_xcb_connection;
      xcb_screen_t* p_xcb_screen;
      xcb_window_t xcb_window;

      xcb_intern_atom_uptr p_xcb_wm_delete_window;

      int default_screen_id;
#endif

      message_handler<const key_event> key_handler;
      message_handler<const mouse_button_event> mouse_button_handler;
      message_handler<const mouse_motion_event> mouse_motion_handler;
      message_handler<const window_close_event> window_close_handler;
      message_handler<const framebuffer_resize_event> framebuffer_resize_handler;

   public:
   };    // class window
} // namespace ui
