#pragma once

#include <vkn/core.hpp>
#include <vkn/device.hpp>

namespace vkn
{
   /**
    * Contains all possible error values comming from the command_pool class.
    */
   enum class command_pool_error
   {
      failed_to_create_command_pool,
      failed_to_allocate_primary_command_buffers,
      failed_to_allocate_secondary_command_buffers
   };

   /**
    * Convert an command_pool_error enum to a string
    */
   auto to_string(command_pool_error err) -> std::string;

   /**
    * A class that wraps around the functionality of a vulkan command pool
    * and maintains command buffers associated with the pool. May only be
    * built using the inner builder class.
    */
   class command_pool final : public owning_handle<vk::CommandPool>
   {
   public:
      /**
       * Get the device used to create the underlying handle
       */
      [[nodiscard]] auto device() const noexcept -> vk::Device;
      [[nodiscard]] auto primary_cmd_buffers() const
         -> const util::dynamic_array<vk::CommandBuffer>&;
      [[nodiscard]] auto secondary_cmd_buffers() const
         -> const util::dynamic_array<vk::CommandBuffer>&;

      [[nodiscard]] auto create_primary_buffer() const noexcept
         -> vkn::result<vk::UniqueCommandBuffer>;

   private:
      std::shared_ptr<util::logger> mp_logger;

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
         builder(const vkn::device& device, std::shared_ptr<util::logger> plogger);

         /**
          * Attempt to build a command_pool object. May return an error
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
         std::shared_ptr<util::logger> mp_logger;

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
   struct is_error_code_enum<vkn::command_pool_error> : true_type
   {
   };
} // namespace std
