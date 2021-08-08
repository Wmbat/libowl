#include <libcacao/surface.hpp>

#include <magic_enum.hpp>

#include <system_error>

namespace cacao
{
   surface::surface(const context& context, vk::SurfaceKHR surface) :
      m_surface(surface, context.instance())
   {}

   auto surface::value() const -> vk::SurfaceKHR { return m_surface.get(); }

   class surface_support_error_category : public std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override { return "cacao_surface_support"; }
      [[nodiscard]] auto message(int err) const -> std::string override
      {
         return std::string(magic_enum::enum_name(static_cast<surface_support_error>(err)));
      }
   };

   inline static const surface_support_error_category surface_support_category{};

   auto to_error_condition(surface_support_error code) -> std::error_condition
   {
      return std::error_condition({static_cast<int>(code), surface_support_category});
   }
} // namespace cacao
