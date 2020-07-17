#include <vkn/device.hpp>

#include <functional>

namespace vkn
{
   namespace detail
   {
      /** QUEUE ERROR CODE */

      auto to_string(queue::error err) -> std::string
      {
         using error = queue::error;
         switch (err)
         {
            case error::compute_unavailable:
               return "compute_unavailable";
            case error::graphics_unavailable:
               return "graphics_unavailable";
            case error::present_unavailable:
               return "present_unavailable";
            case error::transfer_unavailable:
               return "present_unavailable";
            case error::invalid_queue_family_index:
               return "invalid_queue_family_index";
            case error::queue_index_out_of_range:
               return "queue_index_out_of_range";
            default:
               return "UNKNOWN";
         }
      };

      struct queue_error_category : std::error_category
      {
         [[nodiscard]] auto name() const noexcept -> const char* override { return "vk_queue"; }
         [[nodiscard]] auto message(int err) const -> std::string override
         {
            return to_string(static_cast<queue::error>(err));
         }
      };

      static const queue_error_category queue_error_cat{};

      auto make_error_code(queue::error err) -> std::error_code
      {
         return {static_cast<int>(err), queue_error_cat};
      }

      /** DEVICE ERROR CODE */

      auto to_string(device::error err) -> std::string
      {
         using error = device::error;
         switch (err)
         {
            case error::device_extension_not_supported:
               return "device_extension_not_supported";
            case error::failed_to_create_device:
               return "failed_to_create_device";
            default:
               return "UNKNOWN";
         }
      };

      struct device_error_category : std::error_category
      {
         [[nodiscard]] auto name() const noexcept -> const char* override { return "vk_device"; }
         [[nodiscard]] auto message(int err) const -> std::string override
         {
            return to_string(static_cast<device::error>(err));
         }
      };

      const device_error_category device_error_cat;

      auto make_error_code(device::error err) -> std::error_code
      {
         return {static_cast<int>(err), device_error_cat};
      }
   } // namespace detail

   device::device(physical_device physical_device, const create_info& info) :
      m_physical_device{std::move(physical_device)}, m_device{info.device}, m_version{info.version},
      m_extensions{info.extensions}
   {}
   device::device(physical_device physical_device, create_info&& info) :
      m_physical_device{std::move(physical_device)}, m_device{info.device}, m_version{info.version},
      m_extensions{std::move(info.extensions)}
   {}
   device::device(device&& rhs) noexcept { *this = std::move(rhs); }
   device::~device()
   {
      if (m_device)
      {
         m_device.destroy();
      }
   }

   auto device::operator=(device&& rhs) noexcept -> device&
   {
      if (this != &rhs)
      {
         m_physical_device = std::move(rhs.m_physical_device);

         m_device = rhs.m_device;
         rhs.m_device = nullptr;

         m_extensions = std::move(rhs.m_extensions);
      }

      return *this;
   }

   auto device::get_queue_index(queue::type type) const -> vkn::result<uint32_t>
   {
      using err_t = vkn::error;

      if (type == queue::type::present)
      {
         const auto index = detail::get_present_queue_index(m_physical_device.value(),
            m_physical_device.surface(), m_physical_device.queue_families());
         if (!index)
         {
            // clang-format off
            return util::monad::to_error(err_t{
               .type = detail::make_error_code(queue::error::present_unavailable), 
               .result = {}
            });
            // clang-format on
         }
         else
         {
            return util::monad::to_value(index.value_or(0u));
         }
      }
      else if (type == queue::type::graphics)
      {
         if (auto i = detail::get_graphics_queue_index(m_physical_device.queue_families()))
         {
            return util::monad::to_value(i.value());
         }
         else
         {
            // clang-format off
            return util::monad::to_error(err_t{
               .type = detail::make_error_code(queue::error::graphics_unavailable), 
               .result = {}
            });
            // clang-format on
         }
      }
      else if (type == queue::type::compute)
      {
         if (auto i = detail::get_separated_compute_queue_index(m_physical_device.queue_families()))
         {
            return util::monad::to_value(i.value());
         }
         else
         {
            // clang-format off
            return util::monad::to_error(err_t{
               .type = detail::make_error_code(queue::error::compute_unavailable), 
               .result = {}
            });
            // clang-format on
         }
      }
      else if (type == queue::type::transfer)
      {
         if (auto i =
                detail::get_separated_transfer_queue_index(m_physical_device.queue_families()))
         {
            return util::monad::to_value(i.value());
         }
         else
         {
            // clang-format off
            return util::monad::to_error(err_t{
               .type = detail::make_error_code(queue::error::transfer_unavailable), 
               .result = {}
            });
            // clang-format on
         }
      }
      else
      {
         // clang-format off
         return util::monad::to_error(err_t{
            .type = detail::make_error_code(queue::error::invalid_queue_family_index), 
            .result = {}
         });
         // clang-format on
      }
   }

   auto device::get_dedicated_queue_index(queue::type type) const -> vkn::result<uint32_t>
   {
      using err_t = vkn::error;

      if (type == queue::type::compute)
      {
         if (auto i = detail::get_dedicated_compute_queue_index(m_physical_device.queue_families()))
         {
            return util::monad::to_value(i.value());
         }
         else
         {
            // clang-format off
            return util::monad::to_error(err_t{
               .type = detail::make_error_code(queue::error::compute_unavailable), 
               .result = {}
            });
            // clang-format on
         }
      }
      else if (type == queue::type::transfer)
      {
         if (auto i =
                detail::get_dedicated_transfer_queue_index(m_physical_device.queue_families()))
         {
            return util::monad::to_value(i.value());
         }
         else
         {
            // clang-format off
            return util::monad::to_error(err_t{
               .type = detail::make_error_code(queue::error::transfer_unavailable), 
               .result = {}
            });
            // clang-format on
         }
      }
      else
      {
         // clang-format off
         return util::monad::to_error(err_t{
            .type = detail::make_error_code(queue::error::invalid_queue_family_index), 
            .result = {}
         });
         // clang-format on
      }
   }

   auto device::get_queue(queue::type type) const -> vkn::result<vk::Queue>
   {
      using err_t = vkn::error;

      return get_queue_index(type).join(
         [](const err_t& err) -> vkn::result<vk::Queue> {
            return util::monad::to_error(err_t{err});
         },
         [&](uint32_t i) -> vkn::result<vk::Queue> {
            return util::monad::to_value(m_device.getQueue(i, 0));
         });
   }

   auto device::get_dedicated_queue([[maybe_unused]] queue::type type) const -> result<vk::Queue>
   {
      using err_t = vkn::error;

      return get_dedicated_queue_index(type).join(
         [](const err_t& err) -> vkn::result<vk::Queue> {
            return util::monad::to_error(err_t{err});
         },
         [&](uint32_t i) -> vkn::result<vk::Queue> {
            return util::monad::to_value(m_device.getQueue(i, 0));
         });
   }

   auto device::value() const -> const vk::Device& { return m_device; }
   auto device::physical() const -> const physical_device& { return m_physical_device; }

   device::builder::builder(const loader& vk_loader, physical_device&& phys_device,
      uint32_t version, util::logger* plogger) :
      m_loader{vk_loader},
      m_plogger{plogger}
   {
      m_info.phys_device = std::move(phys_device);
      m_info.api_version = version;
   }

   auto device::builder::build() -> result<device>
   {
      using err_t = vkn::error;

      util::dynamic_array<queue::description> descriptions;
      descriptions.insert(descriptions.cend(), m_info.queue_descriptions);

      if (descriptions.empty())
      {
         for (uint32_t i = 0; i < m_info.phys_device.queue_families().size(); ++i)
         {
            // clang-format off
            descriptions.push_back(
               {
                  .index = i, 
                  .count = 1, 
                  .priorities = util::dynamic_array<float>{1.0f}
               }
            );
            // clang-format on
         }
      }

      util::dynamic_array<vk::DeviceQueueCreateInfo> queue_create_infos;
      queue_create_infos.reserve(descriptions.size());
      for (const auto& desc : descriptions)
      {
         // clang-format off
         queue_create_infos.push_back(vk::DeviceQueueCreateInfo{}
            .setPNext(nullptr)
            .setFlags({})
            .setQueueFamilyIndex(desc.index)
            .setQueueCount(desc.count)
            .setPQueuePriorities(desc.priorities.data()));
         // clang-format on    
      }

      util::dynamic_array<const char*> extensions{m_info.desired_extensions};
      if(m_info.phys_device.surface())
      {
         extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
      } 

      vk::PhysicalDevice gpu = m_info.phys_device.value();

      for (const auto& desired : extensions)
      {
         bool is_present = false;
         for (const auto& available : gpu.enumerateDeviceExtensionProperties())
         {
            if (strcmp(desired, static_cast<const char*>(available.extensionName)) == 0)
            {
               is_present = true;
            }
         }

         if (!is_present)
         {
            return util::monad::to_error(err_t{
               .type = detail::make_error_code(device::error::device_extension_not_supported),
               .result={}
            });
         }
      }

      for(const char* name : extensions)
      {
         log_info(m_plogger, "[vkn] device extension: {0} - ENABLED", name);
      }

      // clang-format off
      const auto device_create_info = vk::DeviceCreateInfo{}
         .setPNext(nullptr)
         .setFlags({})
         .setQueueCreateInfoCount(static_cast<uint32_t>(queue_create_infos.size()))
         .setPQueueCreateInfos(queue_create_infos.data())
         .setEnabledLayerCount(0u)
         .setPpEnabledLayerNames(nullptr)
         .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
         .setPpEnabledExtensionNames(extensions.data())
         .setPEnabledFeatures(&m_info.phys_device.features());
      // clang-format on

      auto device_res = util::monad::try_wrap<vk::SystemError>([&] {
         return gpu.createDevice(device_create_info);
      });

      // clang-format off
      if (device_res.is_left())
      {
         return util::monad::to_error(err_t{
            .type = detail::make_error_code(device::error::failed_to_create_device), 
            .result = static_cast<vk::Result>(device_res.left()->code().value())
         });
      }

      log_info(m_plogger, "[vkn] device created");

      auto dev = device_res.right().value();
      m_loader.load_device(dev);

      // clang-format off
      return util::monad::to_value(device{
         std::move(m_info.phys_device), 
         device::create_info{
            .device = dev,
            .version = m_info.api_version,
            .extensions = std::move(extensions)
         }
      });
      // clang-format on
   }

   auto device::builder::set_queue_setup(
      const util::dynamic_array<queue::description>& descriptions) -> device::builder&
   {
      m_info.queue_descriptions = descriptions;
      return *this;
   }
} // namespace vkn
