#pragma once

#include <libcacao/export.hpp>

#include <libmannele/core.hpp>

#include <magic_enum.hpp>

#include <array>
#include <string_view>
#include <system_error>

namespace cacao
{
   enum struct error_code : mannele::u32
   {
      minumum_vulkan_version_not_met,
      window_support_requested_but_not_found,
      no_available_physical_device,
      no_suitable_physical_device,
      queue_type_not_found,
      failed_to_create_window_surface,
      failed_to_create_swapchain,
      failed_to_create_swapchain_image
   };

   struct LIBCACAO_SYMEXPORT error_category : std::error_category
   {
      [[nodiscard]] auto name() const noexcept -> const char* override;
      [[nodiscard]] auto message(int err) const -> std::string override;
   };

   auto LIBCACAO_SYMEXPORT to_error_condition(error_code code) -> std::error_condition;
} // namespace cacao
