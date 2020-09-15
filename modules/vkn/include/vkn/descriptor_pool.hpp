#pragma once

#include <vkn/buffer.hpp>
#include <vkn/device.hpp>

#include <util/strong_type.hpp>

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

      [[nodiscard]] auto sets() const noexcept -> util::dynamic_array<vk::DescriptorSet>;

   private:
      std::shared_ptr<util::logger> mp_logger;

      util::dynamic_array<vk::DescriptorSet> m_sets;

   public:
      class builder final
      {
         using pool_size_dynamic_array =
            util::small_dynamic_array<vk::DescriptorPoolSize, expected_image_count.value()>;

         struct creation_info;

      public:
         builder(const vkn::device& device, std::shared_ptr<util::logger> p_logger) noexcept;

         auto build() -> vkn::result<descriptor_pool>;

         auto set_max_sets(util::count32_t count) noexcept -> builder&;

         auto add_pool_size(vk::DescriptorType type, util::count32_t count) -> builder&;

         auto
         set_descriptor_set_layouts(const util::dynamic_array<vk::DescriptorSetLayout>& layouts)
            -> builder&;
         auto set_descriptor_set_layout(vk::DescriptorSetLayout layout) noexcept -> builder&;

      private:
         [[nodiscard]] auto create_descriptor_pool() const noexcept
            -> vkn::result<vk::UniqueDescriptorPool>;
         auto allocate_descriptor_sets(vk::UniqueDescriptorPool&& handle) const
            -> vkn::result<creation_info>;

      private:
         vk::Device m_device;

         std::shared_ptr<util::logger> mp_logger;

         struct creation_info
         {
            vk::UniqueDescriptorPool pool;
            util::dynamic_array<vk::DescriptorSet> sets;
         };

         struct info
         {
            util::count32_t max_set_count;

            pool_size_dynamic_array pool_sizes;

            vk::DescriptorSetLayout singular_layout;
            util::dynamic_array<vk::DescriptorSetLayout> unique_layouts;
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
