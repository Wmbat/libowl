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
#include <libmannele/logging/log_ptr.hpp>

namespace ash::inline v0
{
   struct device_create_info
   {
      const physical_device& physical;

      mannele::log_ptr logger;
   };

   class device
   {
   public:
      device() = default;
      device(device_create_info&& info);

   private:
      mannele::log_ptr m_logger;

      mannele::semantic_version m_api_version{};

      vk::UniqueDevice m_device;
      std::vector<queue> m_queues;
   };
} // namespace ash::inline v0

#endif // LIBASH_DEVICE_HPP_
