#ifndef LIBCACAO_WINDOW_HPP
#define LIBCACAO_WINDOW_HPP

#include <libcacao/context.hpp>
#include <libcacao/error.hpp>
#include <libcacao/export.hpp>
#include <libcacao/surface.hpp>

#include <libmannele/dimension.hpp>

#include <libreglisse/result.hpp>

#include <GLFW/glfw3.h>

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
      mannele::dimension_i32 dimension = {.width = DEFAULT_WINDOW_WIDTH,
                                          .height = DEFAULT_WINDOW_WIDTH};
      bool is_resizable = false;
   };

   class LIBCACAO_SYMEXPORT window
   {
      using glfw_ptr = std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>>;

   public:
      window() = default;
      window(const window_create_info& info);
      window(window_create_info&& info);

      void poll_events();

      [[nodiscard]] auto create_surface(const cacao::context& context) const
         -> reglisse::result<surface, error_code>;

      [[nodiscard]] auto title() const -> std::string_view;
      [[nodiscard]] auto dimension() const -> const mannele::dimension_i32&;
      [[nodiscard]] auto is_resizable() const -> bool;
      [[nodiscard]] auto is_open() const -> bool;

   private:
      std::string m_title;
      mannele::dimension_i32 m_dimension{};

      bool m_is_resizable{};

      glfw_ptr p_window_handle;
   };
} // namespace cacao

#endif // LIBCACAO_WINDOW_HPP
