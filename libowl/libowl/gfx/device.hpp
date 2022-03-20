/**
 * @file libowl/gfx/device.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date 22nd of January 2022
 * @brief
 * @copyright Copyright (C) 2022 wmbat
 */

#ifndef LIBOWL_GFX_DEVICE_HPP_
#define LIBOWL_GFX_DEVICE_HPP_

#include <libash/device.hpp>
#include <libash/physical_device.hpp>

namespace owl::inline v0
{
   namespace gfx
   {
      class device
      {
      public:
         device(ash::physical_device const* p_physical_device,
                std::span<ash::desired_queue_data const> desired_queues,
                std::span<std::string_view const> desired_extensions, spdlog::logger& logger);

         [[nodiscard]] auto logical() const noexcept -> ash::device const&;
         [[nodiscard]] auto physical() const noexcept -> ash::physical_device const&;

         friend auto operator==(device const& lhs, device const& rhs) noexcept -> bool = default;

      private:
         ash::physical_device const* mp_physical;
         ash::device m_logical;
      };
   } // namespace gfx
} // namespace owl::inline v0

#endif // LIBOWL_GFX_DEVICE_HPP_
