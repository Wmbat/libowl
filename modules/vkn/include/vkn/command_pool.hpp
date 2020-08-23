/**
 * @file command_pool.hpp
 * @author wmbat wmbat@protonmail.com
 * @date 9th of August, 2020
 * @copyright MIT License.
 */

#pragma once

#include "vkn/core.hpp"
#include "vkn/device.hpp"

namespace vkn
{
   /**
    * A class that wraps around the functionality of a vulkan command pool
    * and maintains command buffers associated with the pool.
    */
   class command_pool final
   {
      /**
       * A struct used for error handling and displaying error messages
       */
      struct error_category : std::error_category
      {
         /**
          * The name of the vkn object the error appeared from.
          */
         [[nodiscard]] auto name() const noexcept -> const char* override;
         /**
          * Get the message associated with a specific error code.
          */
         [[nodiscard]] auto message(int err) const -> std::string override;
      };

      inline static const error_category m_category{};

   public:
      /**
       * All necessary data needed to construct a command_pool object.
       */
      struct create_info
      {
         vk::UniqueCommandPool command_pool{nullptr};

         uint32_t queue_index{0};

         util::dynamic_array<vk::CommandBuffer> primary_buffers{};
         util::dynamic_array<vk::CommandBuffer> secondary_buffers{};
      };

      /**
       * Contains all possible error values comming from the command_pool class.
       */
      enum class error_type
      {
         failed_to_create_command_pool,
         failed_to_allocate_primary_command_buffers,
         failed_to_allocate_secondary_command_buffers
      };

      command_pool() = default;
      command_pool(create_info&& info);

      [[nodiscard]] auto value() const noexcept -> vk::CommandPool;
      [[nodiscard]] auto device() const noexcept -> vk::Device;
      [[nodiscard]] auto primary_cmd_buffers() const
         -> const util::dynamic_array<vk::CommandBuffer>&;
      [[nodiscard]] auto secondary_cmd_buffers() const
         -> const util::dynamic_array<vk::CommandBuffer>&;

      /**
       * Transfer an #error_type enum value into a standard error_code.
       */
      inline static auto make_error_code(error_type err) -> std::error_code
      {
         return {static_cast<int>(err), m_category};
      }

   private:
      vk::UniqueCommandPool m_command_pool{nullptr};

      uint32_t m_queue_index{0};

      util::dynamic_array<vk::CommandBuffer> m_primary_buffers;
      util::dynamic_array<vk::CommandBuffer> m_secondary_buffers;

   public:
      /**
       * A class to help in the construction of a command_pool object.
       */
      class builder
      {
      public:
         builder(const vkn::device& device, util::logger* plogger);

         /**
          * Attempt to build a command_pool object.
          */
         auto build() noexcept -> vkn::result<command_pool>;

         /**
          * Set the queue family index for the command pool. All associated command buffers
          * will only work on said queue family index.
          */
         auto set_queue_family_index(uint32_t index) noexcept -> builder&;

         /**
          * Set the number of primary command buffers to build after creating the
          * command_pool
          */
         auto set_primary_buffer_count(uint32_t count) noexcept -> builder&;
         /**
          * Set the number of secondary command buffers to build after creating the
          * command_pool
          */
         auto set_secondary_buffer_count(uint32_t count) noexcept -> builder&;

      private:
         auto create_command_pool(vk::UniqueCommandPool handle) -> vkn::result<command_pool>;
         auto create_primary_buffers(vk::CommandPool pool)
            -> vkn::result<util::dynamic_array<vk::CommandBuffer>>;
         auto create_secondary_buffers(vk::CommandPool handle)
            -> vkn::result<util::dynamic_array<vk::CommandBuffer>>;

      private:
         util::logger* m_plogger;

         struct info
         {
            vk::Device device{nullptr};

            uint32_t queue_family_index{0};

            uint32_t primary_buffer_count{0};
            uint32_t secondary_buffer_count{0};
         } m_info;
      };
   };
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::command_pool::error_type> : true_type
   {
   };
} // namespace std
