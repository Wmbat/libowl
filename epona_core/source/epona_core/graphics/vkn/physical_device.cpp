#include "epona_core/graphics/vkn/physical_device.hpp"

namespace core::gfx::vkn
{
   namespace detail
   {
      std::string to_string(physical_device::error err)
      {
         using error = physical_device::error;
         switch (err)
         {
            case error::failed_to_retrieve_physical_device_count:
               return "failed_to_retrieve_physical_device_count";
            case error::failed_to_enumerate_physical_devices:
               return "failed_to_enumerate_physical_devices";
            case error::no_physical_device_found:
               return "no_physical_device_found";
            case error::no_suitable_device:
               return "no_suitable_device";
            default:
               return "UNKNOWN";
         }
      };

      struct physical_device_error_category : std::error_category
      {
         const char* name() const noexcept override { return "vk_physical_device"; }
         std::string message(int err) const override
         {
            return to_string(static_cast<physical_device::error>(err));
         }
      };

      const physical_device_error_category physical_device_error_cat;

      std::error_code make_error_code(physical_device::error err)
      {
         return {static_cast<int>(err), physical_device_error_cat};
      }
   } // namespace detail

   physical_device::physical_device(physical_device&& rhs) { *this = std::move(rhs); }
   physical_device::~physical_device()
   {
      if (h_instance && h_surface)
      {
         h_instance.destroySurfaceKHR(h_surface);
      }
   }

   physical_device& physical_device::operator=(physical_device&& rhs)
   {
      if (this != &rhs)
      {
         name = std::move(rhs.name);

         features = rhs.features;
         properties = rhs.properties;
         mem_properties = rhs.mem_properties;

         h_instance = std::move(rhs.h_instance);
         rhs.h_instance = vk::Instance{};

         h_device = std::move(rhs.h_device);
         rhs.h_device = vk::PhysicalDevice{};

         h_surface = std::move(rhs.h_surface);
         rhs.h_surface = vk::SurfaceKHR{};

         queue_families = std::move(rhs.queue_families);
      }

      return *this;
   }

   bool physical_device::has_dedicated_compute_queue() const
   {
      return detail::get_dedicated_compute_queue_index(queue_families).has_value();
   }
   bool physical_device::has_dedicated_transfer_queue() const
   {
      return detail::get_dedicated_transfer_queue_index(queue_families).has_value();
   }

   bool physical_device::has_separated_compute_queue() const
   {
      return detail::get_separated_compute_queue_index(queue_families).has_value();
   }
   bool physical_device::has_separated_transfer_queue() const
   {
      return detail::get_separated_transfer_queue_index(queue_families).has_value();
   }

   // physical_device_selector

   physical_device_selector::physical_device_selector(
      const instance& inst, logger* const p_logger) :
      p_logger{p_logger}
   {
      sys_info.instance = inst.h_instance;
      sys_info.instance_extensions = inst.extensions;
   }

   result<physical_device> physical_device_selector::select()
   {
      auto physical_devices_res = monad::try_wrap<std::system_error>([&] {
         return sys_info.instance.enumeratePhysicalDevices();
      });

      if (physical_devices_res.is_left())
      {
         // clang-format off
         return monad::to_left(error{
            .type = detail::make_error_code(
               physical_device::error::failed_to_enumerate_physical_devices), 
            .result = static_cast<vk::Result>(physical_devices_res.left()->code().value())
         });
         // clang-format on
      }

      tiny_dynamic_array<vk::PhysicalDevice, 2> physical_devices =
         std::move(physical_devices_res.right().value());

      tiny_dynamic_array<physical_device_description, 2> physical_device_descriptions;
      for (const auto& device : physical_devices)
      {
         physical_device_descriptions.push_back(populate_device_details(device));
      }

      physical_device_description selected{};
      if (selection_info.select_first_gpu)
      {
         selected = physical_device_descriptions[0];
      }
      else
      {
         for (const auto& desc : physical_device_descriptions)
         {
            const auto suitable = is_device_suitable(desc);
            if (suitable == suitable::yes)
            {
               selected = desc;
               break;
            }
            else if (suitable == suitable::partial)
            {
               selected = desc;
            }
         }
      }

      if (!selected.phys_device)
      {
         // clang-format off
         return monad::to_left(error{
            .type = detail::make_error_code(physical_device::error::no_suitable_device),
            .result = {}
         });
         // clang-format on
      }

      LOG_INFO_P(p_logger, "Selected physical device: {1}", selected.properties.deviceName);

      physical_device gpu{};
      gpu.name = selected.properties.deviceName;
      gpu.features = selected.features;
      gpu.properties = selected.properties;
      gpu.mem_properties = selected.mem_properties;
      gpu.h_instance = sys_info.instance;
      gpu.h_device = selected.phys_device;
      gpu.h_surface = std::move(sys_info.surface);
      gpu.queue_families = selected.queue_families;

      // clang-format off
      return monad::to_right(std::move(gpu));
      // clang-format on
   } // namespace core::gfx::vkn

   physical_device_selector& physical_device_selector::set_prefered_gpu_type(
      physical_device::type type) noexcept
   {
      selection_info.prefered_type = type;
      return *this;
   }

   physical_device_selector& physical_device_selector::set_surface(
      vk::SurfaceKHR&& surface) noexcept
   {
      sys_info.surface = std::move(surface);
      return *this;
   }

   physical_device_selector& physical_device_selector::allow_any_gpu_type(bool allow) noexcept
   {
      selection_info.allow_any_gpu_type = allow;
      return *this;
   }

   physical_device_selector& physical_device_selector::require_present(bool require) noexcept
   {
      selection_info.require_present = require;
      return *this;
   }

   physical_device_selector& physical_device_selector::require_dedicated_compute() noexcept
   {
      selection_info.require_dedicated_compute = true;
      return *this;
   }

   physical_device_selector& physical_device_selector::require_dedicated_transfer() noexcept
   {
      selection_info.require_dedicated_transfer = true;
      return *this;
   }

   physical_device_selector& physical_device_selector::require_separated_compute() noexcept
   {
      selection_info.require_separated_compute = true;
      return *this;
   }

   physical_device_selector& physical_device_selector::require_separated_transfer() noexcept
   {
      selection_info.require_separated_transfer = true;
      return *this;
   }

   physical_device_selector& physical_device_selector::select_first_gpu() noexcept
   {
      selection_info.select_first_gpu = true;
      return *this;
   }

   auto physical_device_selector::populate_device_details(vk::PhysicalDevice device) const noexcept
      -> physical_device_description
   {
      physical_device_description desc{};
      desc.phys_device = device;

      device.getQueueFamilyProperties();

      auto properties_res = monad::try_wrap<vk::SystemError>([&] {
         return device.getQueueFamilyProperties();
      });

      if (!properties_res.is_left())
      {
         desc.queue_families = properties_res.right().value();
      }

      desc.features = device.getFeatures();
      desc.properties = device.getProperties();
      desc.mem_properties = device.getMemoryProperties();

      return desc;
   };

   auto physical_device_selector::is_device_suitable(
      const physical_device_description& desc) const noexcept -> suitable
   {
      if (selection_info.require_dedicated_compute &&
         !detail::get_dedicated_compute_queue_index(desc.queue_families))
      {
         return suitable::no;
      }

      if (selection_info.require_dedicated_transfer &&
         !detail::get_dedicated_transfer_queue_index(desc.queue_families))
      {
         return suitable::no;
      }

      if (selection_info.require_separated_compute &&
         !detail::get_separated_compute_queue_index(desc.queue_families))
      {
         return suitable::no;
      }

      if (selection_info.require_separated_transfer &&
         !detail::get_separated_transfer_queue_index(desc.queue_families))
      {
         return suitable::no;
      }

      if (selection_info.require_present &&
         !detail::get_present_queue_index(desc.phys_device, sys_info.surface, desc.queue_families))
      {
         return suitable::no;
      }

      // clang-format off
      const auto formats = monad::try_wrap<vk::SystemError>([&] {
         return desc.phys_device.getSurfaceFormatsKHR(sys_info.surface);
      }).left_map([](const vk::SystemError&) {
         return dynamic_array<vk::SurfaceFormatKHR>{};
      }).join();

      const auto present_modes = monad::try_wrap<vk::SystemError>([&] {
         return desc.phys_device.getSurfacePresentModesKHR(sys_info.surface);
      }).left_map([](const vk::SystemError&) {
         return dynamic_array<vk::PresentModeKHR>{};
      }).join();
      // clang-format on

      if (formats.empty() || present_modes.empty())
      {
         return suitable::no;
      }

      if (desc.properties.deviceType ==
         static_cast<vk::PhysicalDeviceType>(selection_info.prefered_type))
      {
         return suitable::yes;
      }
      else
      {
         if (selection_info.allow_any_gpu_type)
         {
            return suitable::partial;
         }
         else
         {
            return suitable::no;
         }
      }
   }
} // namespace core::gfx::vkn
