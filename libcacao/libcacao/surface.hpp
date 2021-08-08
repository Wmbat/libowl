#ifndef LIBCACAO_SURFACE_HPP
#define LIBCACAO_SURFACE_HPP

#include <libcacao/context.hpp>
#include <libcacao/export.hpp>

#include <vulkan/vulkan_raii.hpp>

namespace cacao
{
   class LIBCACAO_SYMEXPORT surface
   {
   public:
      surface(const context& context, vk::SurfaceKHR surface);

      [[nodiscard]] auto value() const -> vk::SurfaceKHR;

   private:
      vk::UniqueSurfaceKHR m_surface;
   };

   struct LIBCACAO_SYMEXPORT surface_support
   {
      vk::SurfaceCapabilitiesKHR capabilities;
      std::vector<vk::SurfaceFormatKHR> formats;
      std::vector<vk::PresentModeKHR> present_modes;
   };

   enum class surface_support_error
   {
      failed_to_get_surface_capabilities,
      failed_to_enumerate_formats,
      failed_to_enumerate_present_modes
   };

   auto LIBCACAO_SYMEXPORT to_error_condition(surface_support_error code) -> std::error_condition;
} // namespace cacao

#endif // LIBCACAO_SURFACE_HPP
