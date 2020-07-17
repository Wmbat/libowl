#include <vkn/physical_device.hpp>

namespace vkn
{
   namespace detail
   {
      auto to_string(physical_device::error err) -> std::string
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
         [[nodiscard]] auto name() const noexcept -> const char* override
         {
            return "vk_physical_device";
         }
         [[nodiscard]] auto message(int err) const -> std::string override
         {
            return to_string(static_cast<physical_device::error>(err));
         }
      };

      static const physical_device_error_category physical_device_error_cat{};

      auto make_error_code(physical_device::error err) -> std::error_code
      {
         return {static_cast<int>(err), physical_device_error_cat};
      }
   } // namespace detail

   physical_device::physical_device(const create_info& info) :
      m_name{info.name}, m_features{info.features}, m_properties{info.properties},
      m_mem_properties{info.mem_properties}, m_instance{info.instance}, m_device{info.device},
      m_surface{info.surface}, m_queue_families{info.queue_families}
   {}
   physical_device::physical_device(physical_device&& rhs) noexcept { *this = std::move(rhs); }
   physical_device::~physical_device()
   {
      if (m_instance && m_surface)
      {
         m_instance.destroySurfaceKHR(m_surface);
      }
   }

   auto physical_device::operator=(physical_device&& rhs) noexcept -> physical_device&
   {
      if (this != &rhs)
      {
         m_name = std::move(rhs.m_name);

         m_features = rhs.m_features;
         m_properties = rhs.m_properties;
         m_mem_properties = rhs.m_mem_properties;

         m_instance = rhs.m_instance;
         rhs.m_instance = nullptr;

         m_device = rhs.m_device;
         rhs.m_device = nullptr;

         m_surface = rhs.m_surface;
         rhs.m_surface = nullptr;

         m_queue_families = std::move(rhs.m_queue_families);
      }

      return *this;
   }

   auto physical_device::has_dedicated_compute_queue() const -> bool
   {
      return detail::get_dedicated_compute_queue_index(m_queue_families).has_value();
   }
   auto physical_device::has_dedicated_transfer_queue() const -> bool
   {
      return detail::get_dedicated_transfer_queue_index(m_queue_families).has_value();
   }

   auto physical_device::has_separated_compute_queue() const -> bool
   {
      return detail::get_separated_compute_queue_index(m_queue_families).has_value();
   }
   auto physical_device::has_separated_transfer_queue() const -> bool
   {
      return detail::get_separated_transfer_queue_index(m_queue_families).has_value();
   }

   auto physical_device::value() const noexcept -> const vk::PhysicalDevice& { return m_device; }
   auto physical_device::features() const noexcept -> const vk::PhysicalDeviceFeatures&
   {
      return m_features;
   }
   auto physical_device::surface() const noexcept -> const vk::SurfaceKHR& { return m_surface; }
   auto physical_device::queue_families() const
      -> const util::dynamic_array<vk::QueueFamilyProperties>
   {
      return m_queue_families;
   }
   // physical_device_selector

   using selector = physical_device::selector;

   selector::selector(const instance& inst, util::logger* const plogger) : m_plogger{plogger}
   {
      m_system_info.instance = inst.value();
      m_system_info.instance_extensions = inst.extensions();
   }

   auto selector::select() -> vkn::result<physical_device>
   {
      using err_t = vkn::error;

      const auto physical_devices_res = util::monad::try_wrap<std::system_error>([&] {
         return m_system_info.instance.enumeratePhysicalDevices();
      }).right_map([](const auto& devices) {
         return util::small_dynamic_array<vk::PhysicalDevice, 2>{devices.begin(), devices.end()};
      });

      if (physical_devices_res.is_left())
      {
         // clang-format off
         return util::monad::to_error(err_t{
            .type = detail::make_error_code(
               physical_device::error::failed_to_enumerate_physical_devices), 
            .result = static_cast<vk::Result>(physical_devices_res.left()->code().value())
         });
         // clang-format on
      }

      const auto physical_devices = physical_devices_res.right().value();

      util::small_dynamic_array<physical_device_description, 2> physical_device_descriptions;
      for (const vk::PhysicalDevice& device : physical_devices)
      {
         physical_device_descriptions.push_back(populate_device_details(device));
      }

      physical_device_description selected{};
      if (m_selection_info.select_first_gpu)
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
         return util::monad::to_error(err_t{
            .type = detail::make_error_code(physical_device::error::no_suitable_device),
            .result = {}
         });
         // clang-format on
      }

      log_info(m_plogger, "[vkn] selected physical device: {0}", selected.properties.deviceName);

      // clang-format off
      return util::monad::to_value(physical_device{{
         .name = static_cast<const char*>(selected.properties.deviceName),
         .features = selected.features,
         .properties = selected.properties,
         .mem_properties = selected.mem_properties,
         .instance = m_system_info.instance,
         .device = selected.phys_device,
         .surface = m_system_info.surface,
         .queue_families = util::dynamic_array<vk::QueueFamilyProperties>{
            selected.queue_families.begin(), 
            selected.queue_families.end()
         }
      }});
      // clang-format on
   } // namespace core::gfx::vkn

   auto selector::set_prefered_gpu_type(physical_device::type type) noexcept -> selector&
   {
      m_selection_info.prefered_type = type;
      return *this;
   }

   auto selector::set_surface(vk::SurfaceKHR&& surface) noexcept -> selector&
   {
      m_system_info.surface = surface;
      return *this;
   }

   auto selector::allow_any_gpu_type(bool allow) noexcept -> selector&
   {
      m_selection_info.allow_any_gpu_type = allow;
      return *this;
   }

   auto selector::require_present(bool require) noexcept -> selector&
   {
      m_selection_info.require_present = require;
      return *this;
   }

   auto selector::require_dedicated_compute() noexcept -> selector&
   {
      m_selection_info.require_dedicated_compute = true;
      return *this;
   }

   auto selector::require_dedicated_transfer() noexcept -> selector&
   {
      m_selection_info.require_dedicated_transfer = true;
      return *this;
   }

   auto selector::require_separated_compute() noexcept -> selector&
   {
      m_selection_info.require_separated_compute = true;
      return *this;
   }

   auto selector::require_separated_transfer() noexcept -> selector&
   {
      m_selection_info.require_separated_transfer = true;
      return *this;
   }

   auto selector::select_first_gpu() noexcept -> selector&
   {
      m_selection_info.select_first_gpu = true;
      return *this;
   }

   auto selector::populate_device_details(vk::PhysicalDevice device) const
      -> physical_device_description
   {
      physical_device_description desc{};
      desc.phys_device = device;

      device.getQueueFamilyProperties();

      const auto properties_res = util::monad::try_wrap<vk::SystemError>([&] {
         return device.getQueueFamilyProperties();
      }).right_map([](const auto& properties) {
         return util::small_dynamic_array<vk::QueueFamilyProperties, 16>{
            properties.begin(), properties.end()};
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

   auto selector::is_device_suitable(const physical_device_description& desc) const -> suitable
   {
      if (m_selection_info.require_dedicated_compute &&
         !detail::get_dedicated_compute_queue_index(desc.queue_families))
      {
         return suitable::no;
      }

      if (m_selection_info.require_dedicated_transfer &&
         !detail::get_dedicated_transfer_queue_index(desc.queue_families))
      {
         return suitable::no;
      }

      if (m_selection_info.require_separated_compute &&
         !detail::get_separated_compute_queue_index(desc.queue_families))
      {
         return suitable::no;
      }

      if (m_selection_info.require_separated_transfer &&
         !detail::get_separated_transfer_queue_index(desc.queue_families))
      {
         return suitable::no;
      }

      if (m_selection_info.require_present &&
         !detail::get_present_queue_index(
            desc.phys_device, m_system_info.surface, desc.queue_families))
      {
         return suitable::no;
      }

      // clang-format off
      const auto formats = util::monad::try_wrap<vk::SystemError>([&] {
         return desc.phys_device.getSurfaceFormatsKHR(m_system_info.surface);
      }).left_map([](const vk::SystemError&) {
         return std::vector<vk::SurfaceFormatKHR>{};
      }).join();

      const auto present_modes = util::monad::try_wrap<vk::SystemError>([&] {
         return desc.phys_device.getSurfacePresentModesKHR(m_system_info.surface);
      }).left_map([](const vk::SystemError&) {
         return std::vector<vk::PresentModeKHR>{};
      }).join();
      // clang-format on

      if (formats.empty() || present_modes.empty())
      {
         return suitable::no;
      }

      if (desc.properties.deviceType ==
         static_cast<vk::PhysicalDeviceType>(m_selection_info.prefered_type))
      {
         return suitable::yes;
      }
      else
      {
         if (m_selection_info.allow_any_gpu_type)
         {
            return suitable::partial;
         }
         else
         {
            return suitable::no;
         }
      }
   }
} // namespace vkn
