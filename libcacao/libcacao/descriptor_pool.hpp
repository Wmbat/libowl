/**
 * @file libcacao/descriptor_pool.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Monday, 14th of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBCACAO_DESCRIPTOR_POOL_HPP_
#define LIBCACAO_DESCRIPTOR_POOL_HPP_

#include <libcacao/device.hpp>
#include <libcacao/export.hpp>

// Third Party Libraries

#include <libmannele/logging/log_ptr.hpp>

// Standard Library

#include <vector>

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
      explicit descriptor_pool(const descriptor_pool_create_info& info);

      [[nodiscard]] auto pool() const noexcept -> const vk::DescriptorPool;
      [[nodiscard]] auto sets() const noexcept -> std::span<const vk::DescriptorSet>;

   private:
      vk::UniqueDescriptorPool m_pool{};
      std::vector<vk::DescriptorSet> m_sets{};

      mannele::log_ptr m_logger;
   };
} // namespace cacao

#endif // LIBCACAO_DESCRIPTOR_POOL_HPP_
