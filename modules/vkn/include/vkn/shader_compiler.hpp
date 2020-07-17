/**
 * @file shader_compiler.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 16th of July, 2020
 * @copyright MIT License
 */

#include <vkn/device.hpp>

#include <util/containers/dynamic_array.hpp>
#include <util/logger.hpp>

#include <filesystem>

namespace vkn
{
   class shader_compiler
   {
   public:
      struct create_info
      {
         vk::Device device{};

         util::logger* const plogger{nullptr};
      };

   public:
      shader_compiler(const create_info& info);
      shader_compiler(create_info&& info);

      auto add_shader(const std::filesystem::path& shader_path) -> size_t;

   private:
      util::logger* plogger{nullptr};

      vk::Device m_device;
   };
} // namespace vkn
