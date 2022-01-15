/**
 * @file libash/device.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBASH_DEVICE_HPP_
#define LIBASH_DEVICE_HPP_

#include <libash/physical_device.hpp>
#include <libash/queue.hpp>

#include <libmannele/core/semantic_version.hpp>

#include <spdlog/spdlog.h>

namespace ash::inline v0
{
   struct device_create_info
   {
      const physical_device& physical;

      spdlog::logger& logger;
   };

   class device
   {
   public:
      device() = default;
      device(device_create_info&& info);

      [[nodiscard]] auto api_version() const noexcept -> mannele::semantic_version;

   private:
      spdlog::logger* mp_logger;

      mannele::semantic_version m_api_version{};

      vk::UniqueDevice m_device;
      std::vector<queue> m_queues;
   };
} // namespace ash::inline v0

#endif // LIBASH_DEVICE_HPP_
