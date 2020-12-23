#pragma once

#include <vermillon/vulkan/buffer.hpp>
#include <vermillon/vulkan/device.hpp>

#include <vermillon/util/strong_type.hpp>

namespace vkn
{
   enum struct descriptor_pool_error
   {
      failed_to_create_descriptor_pool,
      failed_to_allocate_descriptor_sets,
      invalid_number_of_descriptor_set_layouts_provided
   };

   class descriptor_pool final : public owning_handle<vk::DescriptorPool>
   {
   public:
      [[nodiscard]] auto device() const noexcept -> vk::Device;

      void update_sets(const vk::DescriptorBufferInfo& info);

      [[nodiscard]] auto sets() const noexcept -> crl::dynamic_array<vk::DescriptorSet>;

   private:
      util::logger_wrapper m_logger;

      crl::dynamic_array<vk::DescriptorSet> m_sets;

   public:
      class builder final
      {
         using pool_size_dynamic_array =
            crl::small_dynamic_array<vk::DescriptorPoolSize, expected_image_count.value()>;

         struct creation_info;

      public:
         builder(const vkn::device& device, util::logger_wrapper logger) noexcept;

         auto build() -> util::result<descriptor_pool>;

         auto set_max_sets(util::count32_t count) noexcept -> builder&;

         auto add_pool_size(vk::DescriptorType type, util::count32_t count) -> builder&;

         auto set_descriptor_set_layouts(const crl::dynamic_array<vk::DescriptorSetLayout>& layouts)
            -> builder&;
         auto set_descriptor_set_layout(vk::DescriptorSetLayout layout) noexcept -> builder&;

      private:
         [[nodiscard]] auto create_descriptor_pool() noexcept
            -> util::result<vk::UniqueDescriptorPool>;
         auto allocate_descriptor_sets(vk::UniqueDescriptorPool&& handle)
            -> util::result<creation_info>;

      private:
         vk::Device m_device;

         util::logger_wrapper m_logger;

         struct creation_info
         {
            vk::UniqueDescriptorPool pool;
            crl::dynamic_array<vk::DescriptorSet> sets;
         };

         struct info
         {
            util::count32_t max_set_count{};

            pool_size_dynamic_array pool_sizes;

            vk::DescriptorSetLayout singular_layout;
            crl::dynamic_array<vk::DescriptorSetLayout> unique_layouts;
         } m_info;
      };
   };

   /**
    * Convert an command_pool_error enum to a string
    */
   auto to_string(descriptor_pool_error err) -> std::string;
   auto to_err_code(descriptor_pool_error err) -> util::error_t;
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::descriptor_pool_error> : true_type
   {
   };
} // namespace std
