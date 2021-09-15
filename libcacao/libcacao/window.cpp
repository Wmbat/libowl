/**
 * @file libcacao/window.cpp
 * @author wmbat wmbat@protonmail.com
 * @date Monday, 14th of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#include <libcacao/window.hpp>

// C++ Standard library

#include <utility>

using reglisse::err;
using reglisse::ok;
using reglisse::result;

namespace cacao
{
   window::window(const window_create_info& info) :
      m_title(info.title), m_dimension(info.dimension), m_is_resizable(info.is_resizable)
   {
      if (!is_glfw_initialized)
      {
         glfwInit();
         is_glfw_initialized = true;
      }

      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, m_is_resizable ? GLFW_TRUE : GLFW_FALSE);

      p_window_handle = glfw_ptr(glfwCreateWindow(static_cast<int>(m_dimension.width),
                                                  static_cast<int>(m_dimension.height),
                                                  m_title.c_str(), nullptr, nullptr),
                                 glfwDestroyWindow);
   }
   window::window(window_create_info&& info) :
      m_title(std::move(info.title)), m_dimension(info.dimension), m_is_resizable(info.is_resizable)
   {
      if (!is_glfw_initialized)
      {
         glfwInit();
         is_glfw_initialized = true;
      }

      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_RESIZABLE, m_is_resizable ? GLFW_TRUE : GLFW_FALSE);

      p_window_handle = glfw_ptr(glfwCreateWindow(static_cast<int>(m_dimension.width),
                                                 static_cast<int>(m_dimension.height),
                                                  m_title.c_str(), nullptr, nullptr),
                                 glfwDestroyWindow);
   }

   void window::poll_events() { glfwPollEvents(); }

   auto window::create_surface(const cacao::context& context) const
      -> result<vk::UniqueSurfaceKHR, error_code>
   {
      VkInstance h_instance = context.instance();
      VkSurfaceKHR h_surface = VK_NULL_HANDLE;

      const VkResult res =
         glfwCreateWindowSurface(h_instance, p_window_handle.get(), nullptr, &h_surface);
      if (res != VK_SUCCESS)
      {
         return err(error_code::failed_to_create_window_surface);
      }

      return ok(vk::UniqueSurfaceKHR(h_surface, context.instance()));
   }

   auto window::title() const -> std::string_view { return m_title; }
   auto window::dimension() const -> const mannele::dimension_u32& { return m_dimension; }
   auto window::is_resizable() const -> bool { return m_is_resizable; }
   auto window::is_open() const -> bool { return !glfwWindowShouldClose(p_window_handle.get()); }
} // namespace cacao
