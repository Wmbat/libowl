#pragma once

#include <vkn/device.hpp>

#include <util/strong_type.hpp>

namespace vkn
{
   enum struct descriptor_pool_error
   {
      failed_to_create_descriptor_pool
   };

   class descriptor_pool final : public owning_handle<vk::DescriptorPool>
   {
   public:
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   public:
      class builder final
      {
         using pool_size_dynamic_array =
            util::small_dynamic_array<vk::DescriptorPoolSize, expected_image_count.value()>;

      public:
         builder(const vkn::device& device, util::logger* p_logger) noexcept;

         auto build() -> vkn::result<descriptor_pool>;

         auto set_max_sets(util::count32_t count) noexcept -> builder&;

         auto add_pool_size(vk::DescriptorType type, util::count32_t count) -> builder&;

      private:
         vk::Device m_device;

         util::logger* mp_logger;

         struct info
         {
            util::count32_t max_set_count;

            pool_size_dynamic_array pool_sizes;
         } m_info;
      };
   };

   /**
    * Convert an command_pool_error enum to a string
    */
   auto to_string(descriptor_pool_error err) -> std::string;
   /**
    * Convert an descriptor_pool_error enum value and an error code from a vulkan error into
    * a vkn::error
    */
   auto make_error(descriptor_pool_error err, std::error_code ec) -> vkn::error;
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::descriptor_pool_error> : true_type
   {
   };
} // namespace std
