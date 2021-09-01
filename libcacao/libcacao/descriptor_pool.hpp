#ifndef LIBCACAO_DESCRIPTOR_POOL_HPP
#define LIBCACAO_DESCRIPTOR_POOL_HPP

#include <libcacao/device.hpp>
#include <libcacao/export.hpp>

#include <libmannele/logging/log_ptr.hpp>

namespace cacao
{
   struct descriptor_pool_create_info
   {
      const cacao::device& device;

      std::vector<vk::DescriptorPoolSize> pool_sizes;
      std::vector<vk::DescriptorSetLayout> layouts;

      mannele::log_ptr logger;
   };

   class descriptor_pool
   {
   public:
      descriptor_pool() = default;
      descriptor_pool(const descriptor_pool_create_info& info);

      [[nodiscard]] auto pool() const noexcept -> const vk::DescriptorPool;
      [[nodiscard]] auto sets() const noexcept -> std::span<const vk::DescriptorSet>;

   private:
      vk::UniqueDescriptorPool m_pool{};
      std::vector<vk::DescriptorSet> m_sets{};

      mannele::log_ptr m_logger;
   };
} // namespace cacao

#endif // LIBCACAO_DESCRIPTOR_POOL_HPP
