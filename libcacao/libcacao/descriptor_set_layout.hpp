#ifndef LIBCACAO_DESCRIPTOR_SET_LAYOUT
#define LIBCACAO_DESCRIPTOR_SET_LAYOUT

#include <libcacao/device.hpp>

#include <span>

namespace cacao 
{
   struct descriptor_set_layout_create_info
   {
      const cacao::device& device;

      std::vector<vk::DescriptorSetLayoutBinding> bindings;

      util::log_ptr logger;
   };

   class descriptor_set_layout
   {
   public:
      descriptor_set_layout() = default;
      descriptor_set_layout(descriptor_set_layout_create_info&& info);

      [[nodiscard]] auto value() const -> vk::DescriptorSetLayout;
      [[nodiscard]] auto bindings() const -> std::span<const vk::DescriptorSetLayoutBinding>;

   private:
      std::vector<vk::DescriptorSetLayoutBinding> m_bindings;

      vk::UniqueDescriptorSetLayout m_set_layout;
   };
} // namespace cacao

#endif // LIBCACAO_DESCRIPTOR_SET_LAYOUT
