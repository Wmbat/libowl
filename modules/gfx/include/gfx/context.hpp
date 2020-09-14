#pragma once

#include <util/logger.hpp>
#include <util/strong_type.hpp>

#include <vkn/core.hpp>
#include <vkn/instance.hpp>

namespace gfx
{
   class context
   {
   public:
      context(std::shared_ptr<util::logger> p_logger);

      [[nodiscard]] auto vulkan_loader() const noexcept -> const vkn::loader&;
      [[nodiscard]] auto vulkan_instance() const noexcept -> const vkn::instance&;

   private:
      [[nodiscard]] auto create_instance() const noexcept -> vkn::instance;

   private:
      std::shared_ptr<util::logger> mp_logger;

      vkn::loader m_vulkan_loader;
      vkn::instance m_vulkan_instance;

      inline static std::uint32_t m_context_counter;
      static constexpr std::uint32_t m_max_context_allowed = 1u;
   };
} // namespace gfx
