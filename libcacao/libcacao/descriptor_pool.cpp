#include <libcacao/descriptor_pool.hpp>

namespace cacao
{
   descriptor_pool::descriptor_pool(const descriptor_pool_create_info& info) :
      m_pool(info.device.logical().createDescriptorPoolUnique(
         {.maxSets = static_cast<mannele::u32>(std::size(info.layouts)),
          .poolSizeCount = static_cast<mannele::u32>(std::size(info.pool_sizes)),
          .pPoolSizes = std::data(info.pool_sizes)})),
      m_sets(info.device.logical().allocateDescriptorSets(vk::DescriptorSetAllocateInfo{
         .descriptorPool = m_pool.get(),
         .descriptorSetCount = static_cast<mannele::u32>(std::size(info.layouts)),
         .pSetLayouts = std::data(info.layouts)})),
      m_logger(info.logger)
   {}

   auto descriptor_pool::pool() const noexcept -> const vk::DescriptorPool { return m_pool.get(); }
   auto descriptor_pool::sets() const noexcept -> std::span<const vk::DescriptorSet>
   {
      return m_sets;
   }

} // namespace cacao
