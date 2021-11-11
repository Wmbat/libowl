/**
 * @file libcacao/window.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date Monday, 14th of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBCACAO_WINDOW_HPP_
#define LIBCACAO_WINDOW_HPP_

#include <libcacao/context.hpp>
#include <libcacao/error.hpp>
#include <libcacao/export.hpp>

// Third Party Libraries

#include <GLFW/glfw3.h>

#include <libmannele/dimension.hpp>

#include <libreglisse/result.hpp>

// C++ Standard Library

#include <functional>
#include <memory>
#include <string>

namespace cacao
{
   static constexpr mannele::i32 DEFAULT_WINDOW_WIDTH = 1080;
   static constexpr mannele::i32 DEFAULT_WINDOW_HEIGHT = 720;

   struct window_create_info
   {
      std::string title = "Default libcacao window";
      mannele::dimension_u32 dimension = {.width = DEFAULT_WINDOW_WIDTH,
                                          .height = DEFAULT_WINDOW_WIDTH};
      bool is_resizable = false;
   };

   class LIBCACAO_SYMEXPORT window
   {
      using glfw_ptr = std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>>;

   public:
      window() = default;
      explicit window(const window_create_info& info);
      explicit window(window_create_info&& info);

      void poll_events();

      [[nodiscard]] auto create_surface(const cacao::context& context) const
         -> reglisse::result<vk::UniqueSurfaceKHR, error_code>;

      [[nodiscard]] auto title() const -> std::string_view;
      [[nodiscard]] auto dimension() const -> const mannele::dimension_u32&;
      [[nodiscard]] auto is_resizable() const -> bool;
      [[nodiscard]] auto is_open() const -> bool;

   private:
      std::string m_title;
      mannele::dimension_u32 m_dimension{};

      bool m_is_resizable{};

      glfw_ptr p_window_handle;

      inline static bool is_glfw_initialized = false; // NOLINT
   };
} // namespace cacao

#endif // LIBCACAO_WINDOW_HPP_
