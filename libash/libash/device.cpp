#include <libash/device.hpp>

namespace ash::inline v0
{
   device::device(device_create_info&& info) : m_logger(info.logger)
   {
      const auto& features = info.physical.features;

      m_device = info.physical.device.createDeviceUnique(
         vk::DeviceCreateInfo().setPEnabledFeatures(&features));
   }

} // namespace ash::inline v0
