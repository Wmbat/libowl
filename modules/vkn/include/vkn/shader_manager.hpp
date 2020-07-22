/**
 * @file shader_manager.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 16th of July, 2020
 * @copyright MIT License
 */

#pragma once

#include <vkn/device.hpp>
#include <vkn/shader.hpp>

#include <util/containers/dense_hash_map.hpp>
#include <util/logger.hpp>

#include <filesystem>

namespace vkn
{
   class shader_manager
   {
   public:
      struct create_info
      {
         vk::Device device{};

         util::logger* const plogger{nullptr};
      };

   public:
      shader_manager(const create_info& info);
      shader_manager(create_info&& info);

      auto add_shader(const std::filesystem::path& shader_path) -> uint32_t;

   private:
      util::logger* plogger{nullptr};

      vk::Device m_device;

      util::dense_hash_map<uint32_t, shader> shaders_;
   };
} // namespace vkn
