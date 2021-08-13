#pragma once

#include <sph-simulation/core.hpp>

#include <vulkan/vulkan_raii.hpp>

class framebuffer
{
public:
   /**
    * @brief Data used for the creation of a framebuffer object.
    */
   struct create_info
   {
      vk::Device device{};
      vk::RenderPass pass{};

      std::vector<vk::ImageView> attachments{};

      std::uint32_t width{};
      std::uint32_t height{};
      std::uint32_t layers{};

      util::log_ptr logger;
   };

public:
   /**
    * @brief Creates a framebuffer object.
    *
    * @param info
    *
    * @throws If something failed during construction
    */
   framebuffer(create_info&& info);

   /**
    * @brief access the underlying vulkan handle
    *
    * @return The vulkan `vk::framebuffer` handle.
    */
   auto value() const -> vk::Framebuffer; // NOLINT

private:
   vk::UniqueFramebuffer m_framebuffer;

   std::uint32_t m_width;
   std::uint32_t m_height;
   std::uint32_t m_layers;
};
