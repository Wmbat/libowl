#pragma once

#include <fluid_simulation/core.hpp>

#include <vermillon/vulkan/core.hpp>

#include <libcaramel/containers/dynamic_array.hpp>

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

      crl::dynamic_array<vk::ImageView> attachments{};

      std::uint32_t width{};
      std::uint32_t height{};
      std::uint32_t layers{};

      util::logger_wrapper logger;
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
