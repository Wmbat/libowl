#include <gfx/context.hpp>

#include <cassert>

namespace gfx
{
   context::context(std::shared_ptr<util::logger> p_logger) :
      mp_logger{std::move(p_logger)}, m_vulkan_loader{mp_logger}
   {
      if (++m_context_counter > m_max_context_allowed)
      {
         assert(false && "A context has already been created");
      }

      m_vulkan_instance = create_instance();
   }

   auto context::vulkan_loader() const noexcept -> const vkn::loader& { return m_vulkan_loader; }
   auto context::vulkan_instance() const noexcept -> const vkn::instance&
   {
      return m_vulkan_instance;
   }

   auto context::create_instance() const noexcept -> vkn::instance
   {
      return vkn::instance::builder{m_vulkan_loader, mp_logger}
         .set_application_name("")
         .set_application_version(0, 0, 0)
         .set_engine_name("melodie")
         .set_engine_version(0, 0, 0)
         .build()
         .map_error([&](auto&& err) {
            log_error(mp_logger, "[core] Failed to create instance: {0}", err.type.message());
            std::terminate();

            return vkn::instance{};
         })
         .join();
   }
} // namespace gfx
