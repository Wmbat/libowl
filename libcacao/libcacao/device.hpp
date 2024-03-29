/**
 * @file libcacao/device.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date Monday, 14th of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBCACAO_DEVICE_HPP_
#define LIBCACAO_DEVICE_HPP_

#include <libcacao/context.hpp>
#include <libcacao/runtime_error.hpp>
#include <libcacao/util/flags.hpp>

// Third Party Libraries

#include <libreglisse/maybe.hpp>

// C++ Standard Library

#include <functional>
#include <string>
#include <vector>

namespace cacao
{
   enum struct queue_flag_bits : std::uint32_t
   {
      none = 0 << 0,
      graphics = 1 << 0,
      present = 1 << 1,
      compute = 1 << 2,
      transfer = 1 << 3
   };

   using queue_flags = flags<queue_flag_bits>;

   template <>
   struct flag_traits<queue_flag_bits>
   {
      enum : std::uint32_t
      {
         all = static_cast<std::uint32_t>(queue_flag_bits::graphics) |
            static_cast<std::uint32_t>(queue_flag_bits::present) |
            static_cast<std::uint32_t>(queue_flag_bits::compute) |
            static_cast<std::uint32_t>(queue_flag_bits::transfer)
      };
   };

   DEFINE_EXTRA_ENUM_OPERATORS(queue_flags);

   auto to_string(const queue_flags& value) -> std::string;

   namespace detail
   {
      auto to_queue_flag_bits(const vk::QueueFlags& flags) -> queue_flags;
   } // namespace detail

   auto rate_physical_device(vk::PhysicalDevice device) -> std::int32_t;

   struct device_create_info
   {
      const context& ctx;

      vk::SurfaceKHR surface = nullptr;

      std::function<std::int32_t(vk::PhysicalDevice)> physical_device_rating_fun{
         rate_physical_device};

      bool use_transfer_queue = false;
      bool use_compute_queue = false;

      mannele::log_ptr logger;
   };

   struct queue
   {
      vk::Queue value;
      queue_flags type{};
      std::uint32_t family_index{};
   };

   class device
   {
   public:
      device() = default;
      explicit device(device_create_info&& info);

      [[nodiscard]] auto logical() const -> vk::Device;
      [[nodiscard]] auto physical() const -> vk::PhysicalDevice;

      [[nodiscard]] auto vk_version() const -> std::uint32_t;

      [[nodiscard]] auto find_best_suited_queue(const queue_flags& flags) const -> queue;

   private:
      [[nodiscard]] auto find_physical_device(const device_create_info& info) const
         -> vk::PhysicalDevice;
      [[nodiscard]] auto create_logical_device(const device_create_info& info) const
         -> vk::UniqueDevice;
      [[nodiscard]] auto create_queues(const device_create_info& info) const -> std::vector<queue>;

   private:
      vk::PhysicalDevice m_physical;
      vk::UniqueDevice m_logical;

      std::uint32_t m_vk_version{};

      std::vector<queue> m_queues{};
   };
} // namespace cacao

#endif // LIBCACAO_DEVICE_HPP_
