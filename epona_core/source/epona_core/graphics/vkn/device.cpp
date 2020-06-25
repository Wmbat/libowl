#include "epona_core/graphics/vkn/device.hpp"

#include <functional>

namespace core::gfx::vkn
{
   namespace detail
   {
      /** QUEUE ERROR CODE */

      std::string to_string(queue::error err)
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
         const char* name() const noexcept override { return "vk_queue"; }
         std::string message(int err) const override
         {
            return to_string(static_cast<queue::error>(err));
         }
      };

      const queue_error_category queue_error_cat;

      std::error_code make_error_code(queue::error err)
      {
         return {static_cast<int>(err), queue_error_cat};
      }

      /** DEVICE ERROR CODE */

      std::string to_string(device::error err)
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
         const char* name() const noexcept override { return "vk_device"; }
         std::string message(int err) const override
         {
            return to_string(static_cast<device::error>(err));
         }
      };

      const device_error_category device_error_cat;

      std::error_code make_error_code(device::error err)
      {
         return {static_cast<int>(err), device_error_cat};
      }
   } // namespace detail

   device::device(device&& rhs) { *this = std::move(rhs); }
   device::~device()
   {
      if (h_device)
      {
         h_device.destroy();
      }
   }

   device& device::operator=(device&& rhs)
   {
      if (this != &rhs)
      {
         phys_device = std::move(rhs.phys_device);
         h_device = std::move(rhs.h_device);
         rhs.h_device = vk::Device{};

         extensions = std::move(rhs.extensions);
      }

      return *this;
   }

   device&& device::set_physical_device(physical_device&& phys)
   {
      phys_device = std::move(phys);
      return std::move(*this);
   }
   device&& device::set_device(vk::Device&& device)
   {
      h_device = std::move(device);
      return std::move(*this);
   }
   device&& device::set_extensions(dynamic_array<const char*> exts)
   {
      extensions = exts;
      return std::move(*this);
   }

   result<uint32_t> device::get_queue_index(queue::type type) const
   {
      using err_t = core::gfx::vkn::error;

      if (type == queue::type::present)
      {
         const auto index = detail::get_present_queue_index(
            phys_device.h_device, phys_device.h_surface, phys_device.queue_families);
         if (!index)
         {
            // clang-format off
            return monad::to_left(err_t{
               .type = detail::make_error_code(queue::error::present_unavailable), 
               .result = {}
            });
            // clang-format on
         }
         else
         {
            return monad::to_right(index.value_or(0u));
         }
      }
      else if (type == queue::type::graphics)
      {
         const auto index = detail::get_graphics_queue_index(phys_device.queue_families);
         if (!index)
         {
            // clang-format off
            return monad::to_left(err_t{
               .type = detail::make_error_code(queue::error::graphics_unavailable), 
               .result = {}
            });
            // clang-format on
         }
         else
         {
            return monad::to_right(index.value_or(0u));
         }
      }
      else if (type == queue::type::compute)
      {
         const auto index = detail::get_separated_compute_queue_index(phys_device.queue_families);
         if (!index)
         {
            // clang-format off
            return monad::to_left(err_t{
               .type = detail::make_error_code(queue::error::compute_unavailable), 
               .result = {}
            });
            // clang-format on
         }
         else
         {
            return monad::to_right(index.value_or(0u));
         }
      }
      else if (type == queue::type::transfer)
      {
         const auto index = detail::get_separated_transfer_queue_index(phys_device.queue_families);
         if (!index)
         {
            // clang-format off
            return monad::to_left(err_t{
               .type = detail::make_error_code(queue::error::transfer_unavailable), 
               .result = {}
            });
            // clang-format on
         }
         else
         {
            return monad::to_right(index.value_or(0u));
         }
      }
      else
      {
         // clang-format off
         return monad::to_left(err_t{
            .type = detail::make_error_code(queue::error::invalid_queue_family_index), 
            .result = {}
         });
         // clang-format on
      }
   }

   result<uint32_t> device::get_dedicated_queue_index(queue::type type) const
   {
      using err_t = core::gfx::vkn::error;

      if (type == queue::type::compute)
      {
         const auto index = detail::get_dedicated_compute_queue_index(phys_device.queue_families);
         if (!index)
         {
            // clang-format off
            return monad::to_left(err_t{
               .type = detail::make_error_code(queue::error::compute_unavailable), 
               .result = {}
            });
            // clang-format on
         }
         else
         {
            return monad::to_right(index.value_or(0u));
         }
      }
      else if (type == queue::type::transfer)
      {
         const auto index = detail::get_dedicated_transfer_queue_index(phys_device.queue_families);
         if (!index)
         {
            // clang-format off
            return monad::to_left(err_t{
               .type = detail::make_error_code(queue::error::transfer_unavailable), 
               .result = {}
            });
            // clang-format on
         }
         else
         {
            return monad::to_right(index.value_or(0u));
         }
      }
      else
      {
         // clang-format off
         return monad::to_left(err_t{
            .type = detail::make_error_code(queue::error::invalid_queue_family_index), 
            .result = {}
         });
         // clang-format on
      }
   }

   result<vk::Queue> device::get_queue(queue::type type) const
   {
      using err_t = core::gfx::vkn::error;

      return get_queue_index(type).join(
         [](const err_t& err) -> result<vk::Queue> {
            return monad::to_left(err_t{err});
         },
         [&](uint32_t i) -> result<vk::Queue> {
            return monad::to_right(h_device.getQueue(i, 0));
         });
   }

   result<vk::Queue> device::get_dedicated_queue([[maybe_unused]] queue::type type) const
   {
      using err_t = core::gfx::vkn::error;

      return get_dedicated_queue_index(type).join(
         [](const err_t& err) -> result<vk::Queue> {
            return monad::to_left(err_t{err});
         },
         [&](uint32_t i) -> result<vk::Queue> {
            return monad::to_right(h_device.getQueue(i, 0));
         });
   }

   device_builder::device_builder(
      const loader& vk_loader, physical_device&& phys_device, logger* p_logger) :
      vk_loader{vk_loader},
      p_logger{p_logger}
   {
      info.phys_device = std::move(phys_device);
   }

   result<device> device_builder::build()
   {
      dynamic_array<queue::description> descriptions;
      descriptions.insert(descriptions.cend(), info.queue_descriptions);

      if (descriptions.empty())
      {
         for (uint32_t i = 0; i < info.phys_device.queue_families.size(); ++i)
         {
            // clang-format off
            descriptions.push_back(
               {
                  .index = i, 
                  .count = 1, 
                  .priorities = dynamic_array<float>{1.0f}
               }
            );
            // clang-format on
         }
      }

      dynamic_array<vk::DeviceQueueCreateInfo> queue_create_infos;
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

      tiny_dynamic_array<const char*, 4> extensions{info.desired_extensions};
      if(info.phys_device.h_surface)
      {
         extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
      } 

      vk::PhysicalDevice vk_gpu = info.phys_device.h_device;

      for (const auto& desired : extensions)
      {
         bool is_present = false;
         for (const auto& available : vk_gpu.enumerateDeviceExtensionProperties())
         {
            if (strcmp(desired, available.extensionName) == 0)
            {
               is_present = true;
            }
         }

         if (!is_present)
         {
            return monad::to_left(error{
               .type = detail::make_error_code(device::error::device_extension_not_supported),
               .result={}
            });
         }
      }

      for(const char* name : extensions)
      {
         LOG_INFO_P(p_logger, "Device extension: {1} - ENABLED", name);
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
         .setPEnabledFeatures(&info.phys_device.features);
      // clang-format on

      auto device_res = monad::try_wrap<vk::SystemError>([&] {
         return vk_gpu.createDevice(device_create_info);
      });

      // clang-format off
      if (device_res.is_left())
      {
         return monad::to_left(error{
            .type = detail::make_error_code(device::error::failed_to_create_device), 
            .result = static_cast<vk::Result>(device_res.left()->code().value())
         });
      }

      auto device = vkn::device{}
         .set_device(device_res.right().value())
         .set_physical_device(std::move(info.phys_device))
         .set_extensions(extensions);
      // clang-format on

      LOG_INFO(p_logger, "vk - device created");

      vk_loader.load_device(device.h_device);

      return monad::to_right(std::move(device));
   }

   device_builder& device_builder::set_queue_setup(
      const range_over<queue::description> auto& descriptions)
   {
      info.queue_descriptions = descriptions;
      return *this;
   }
} // namespace core::gfx::vkn
