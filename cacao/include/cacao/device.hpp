#pragma once

#include <cacao/context.hpp>
#include <cacao/util/error.hpp>
#include <cacao/util/flags.hpp>

#include <monads/maybe.hpp>

#include <functional>
#include <string>

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

      struct queue_info
      {
         queue_flags flag_bits{};
         std::uint32_t family{};
         crl::small_dynamic_array<float, 1> priorities;
      };
   } // namespace detail

   auto rate_physical_device(vk::PhysicalDevice device) -> std::int32_t;

   struct device_create_info
   {
      const context& ctx;

      vk::SurfaceKHR surface{nullptr};

      std::function<std::int32_t(vk::PhysicalDevice)> physical_device_rating_fun{
         rate_physical_device};

      bool use_transfer_queue = false;
      bool use_compute_queue = false;

      util::logger_wrapper logger;
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
      device(device_create_info&& info);

      [[nodiscard]] auto logical() const -> vk::Device;
      [[nodiscard]] auto physical() const -> vk::PhysicalDevice;

      [[nodiscard]] auto vk_version() const -> std::uint32_t;

      auto get_queue_index(const queue_flags& type) const -> std::uint32_t;
      auto get_queue(const queue_flags& type) const -> const queue&;

   private:
      auto find_physical_device(const device_create_info& info) const -> vk::PhysicalDevice;
      auto create_logical_device(const device_create_info& info) const -> vk::UniqueDevice;
      auto create_queues(const device_create_info& info) const
         -> crl::small_dynamic_array<queue, 2>;

      auto get_queue_create_infos(const device_create_info& info) const
         -> const crl::dynamic_array<detail::queue_info>;

   private:
      vk::PhysicalDevice m_physical;
      vk::UniqueDevice m_logical;

      std::uint32_t m_vk_version{};

      crl::small_dynamic_array<queue, 2> m_queues;
   };

   auto get_dedicated_queue_index(const queue_flags& flags, std::span<const queue> queues)
      -> monad::maybe<std::uint32_t>;
   auto get_specialized_queue_index(const queue_flags& desired, const queue_flags& unwanted,
                                    std::span<const queue> queues) -> monad::maybe<std::uint32_t>;
   auto get_queue_index(const queue_flags& desired, std::span<const queue> queues)
      -> monad::maybe<std::uint32_t>;
} // namespace cacao
