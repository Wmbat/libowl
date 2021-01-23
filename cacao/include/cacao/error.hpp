#pragma once

#include <cacao/util/error.hpp>

#include <array>
#include <string_view>

namespace cacao
{
   enum struct error_code : std::uint32_t
   {
      minumum_vulkan_version_not_met = 0,
      window_support_requested_but_not_found = 1,
      no_available_physical_device = 2,
      no_suitable_physical_device = 3,
      queue_type_not_found = 4,
   };

   constexpr std::array error_code_names = {
      "minumum_vulkan_version_not_met", "window_support_requested_but_not_found",
      "no_available_physical_device", "no_suitable_physical_device", "queue_type_not_found"};

   constexpr auto to_string(error_code code) -> std::string_view
   {
      return error_code_names.at(static_cast<std::size_t>(code));
   }

   namespace detail
   {
      struct cacao_error_category : std::error_category
      {
         [[nodiscard]] auto name() const noexcept -> const char* override { return "cacao"; }
         [[nodiscard]] auto message(int err) const -> std::string override
         {
            return std::string{to_string(static_cast<error_code>(err))};
         }
      };

      static const cacao_error_category cacao_error_cat{};
   } // namespace detail

   inline auto to_error_cond(error_code code) -> error_t
   {
      return {{static_cast<int>(code), detail::cacao_error_cat}};
   }
} // namespace cacao
