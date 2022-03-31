
#include <libowl/gfx/device.hpp>
#include <libowl/runtime_error.hpp>

namespace owl::inline v0
{
   namespace gfx
   {
      device::device(ash::physical_device const* p_physical_device,
                     std::span<ash::desired_queue_data const> desired_queues,
                     std::span<std::string_view const> desired_extensions, spdlog::logger& logger) :
         mp_physical(p_physical_device),
         m_logical({.physical = *mp_physical,
                    .desired_queues = desired_queues,
                    .extensions = desired_extensions,
                    .logger = logger})
      {}

      [[nodiscard]] auto device::logical() const noexcept -> ash::device const&
      {
         return m_logical;
      }
      [[nodiscard]] auto device::physical() const noexcept -> ash::physical_device const&
      {
         return *mp_physical;
      }
   } // namespace gfx
} // namespace owl::inline v0

