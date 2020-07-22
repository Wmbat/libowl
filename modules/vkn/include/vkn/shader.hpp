/**
 * @file shader.hpp
 * @author wmbat wmbat@protonmail.com
 * @date Saturday, 18th of July, 2020
 * @copyright MIT License
 */

#pragma once

#include <vkn/device.hpp>

#include <util/logger.hpp>

#include <filesystem>

namespace vkn
{
   class shader
   {
      struct error_category : std::error_category
      {
         [[nodiscard]] auto name() const noexcept -> const char* override;
         [[nodiscard]] auto message(int err) const -> std::string override;
      };

   public:
      enum class error
      {
         invalid_filepath,
         filepath_not_a_file,
         failed_to_open_file
      };

      struct create_info
      {
         vk::Device device;
         vk::ShaderModule shader_module;
      };

      enum class type
      {
         vertex,
         fragment,
         compute,
         geometry
      };

   public:
      shader(const create_info& info);
      shader(create_info&& info);

      auto value() noexcept -> vk::ShaderModule&;
      [[nodiscard]] auto value() const noexcept -> const vk::ShaderModule&;

      inline static auto make_error_code(error err) -> std::error_code
      {
         return {static_cast<int>(err), m_category};
      }

   private:
      vk::Device m_device;
      vk::ShaderModule m_shader_module;

      type m_type;

      inline static const error_category m_category{};

   public:
      class builder
      {
      public:
         builder(vk::Device device, util::logger* const plogger);

         auto build() -> result<shader>;

         auto set_filepath(const std::filesystem::path& path);

      private:
         util::logger* const m_plogger{nullptr};

         struct info
         {
            vk::Device device;

            std::filesystem::path path;
         } m_info;
      };
   };
} // namespace vkn
