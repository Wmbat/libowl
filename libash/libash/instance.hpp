#ifndef LIBASH_INSTANCE_HPP_
#define LIBASH_INSTANCE_HPP_

#include <libash/core.hpp>
#include <libash/detail/vulkan.hpp>

#include <libmannele/core/semantic_version.hpp>
#include <libmannele/logging/log_ptr.hpp>

#include <span>
#include <string_view>

namespace ash::detail
{
   auto is_layer_available(std::string_view name,
                           std::span<const vk::LayerProperties> available_layers) -> bool;
   auto is_extension_available(std::span<const vk::ExtensionProperties> extensions,
                               std::string_view name) -> bool;
} // namespace ash::detail

namespace ash
{
   enum struct instance_error
   {
      window_support_not_found,
      layer_support_not_found,
      extension_support_not_found
   };

   auto to_error_condition(instance_error err) -> std::error_condition;

   struct application_info
   {
      std::string_view name;
      mannele::semantic_version version;
   };

   struct engine_info
   {
      std::string_view name;
      mannele::semantic_version version;
   };

   struct instance_create_info
   {
      application_info app_info;
      engine_info engine_info;

      bool is_headless = false;

      std::vector<const char*> enabled_extension_names;
      std::vector<const char*> enabled_layer_names;

      mannele::log_ptr logger;
   };

   class instance
   {
   public:
      instance(const instance_create_info& info);

      operator vk::Instance();

      [[nodiscard]] auto version() const noexcept -> u32;

   private:
      vk::DynamicLoader m_loader{};

      u32 m_api_version{};

      vk::UniqueInstance m_instance{nullptr};
      vk::UniqueDebugUtilsMessengerEXT m_debug_utils{nullptr};

      mannele::log_ptr m_logger;
   };
} // namespace ash

#endif // LIBASH_INSTANCE_HPP_
