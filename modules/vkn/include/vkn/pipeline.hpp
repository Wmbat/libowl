#pragma once

#include <vkn/core.hpp>
#include <vkn/device.hpp>
#include <vkn/render_pass.hpp>
#include <vkn/shader.hpp>

namespace vkn
{
   class graphics_pipeline final
   {
      static constexpr std::size_t expected_shader_count{2u};

   public:
      using value_type = vk::Pipeline;
      using pointer = value_type*;
      using const_pointer = const value_type*;

      enum struct error
      {
         failed_to_create_pipeline_layout,
         failed_to_create_pipeline
      };

   public:
      graphics_pipeline() = default;

      /**
       * Allow direct access to the underlying handle functions
       */
      auto operator->() noexcept -> pointer;
      /**
       * Allow direct access to the underlying handle functions
       */
      auto operator->() const noexcept -> const_pointer;

      /**
       * Get the underlying handle
       */
      auto operator*() const noexcept -> value_type;

      operator bool() const noexcept;

      /**
       * Get the underlying handle
       */
      [[nodiscard]] auto value() const noexcept -> value_type;
      [[nodiscard]] auto layout() const noexcept -> vk::PipelineLayout;
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   private:
      vk::UniquePipeline m_pipeline{nullptr};
      vk::UniquePipelineLayout m_pipeline_layout{nullptr};

   public:
      class builder final
      {
         template <typename any_>
         using shader_dynamic_array = util::small_dynamic_array<any_, expected_shader_count>;

      public:
         builder(const vkn::device& device, const vkn::render_pass& render_pass,
                 util::logger* plogger);

         auto build() -> vkn::result<graphics_pipeline>;

         auto add_shader(const vkn::shader& shader) noexcept -> builder&;
         auto add_viewport(const vk::Viewport& viewport, const vk::Rect2D& scissor) noexcept
            -> builder&;

         auto set_topology(vk::PrimitiveTopology topology) noexcept -> builder&;

         auto enable_primitive_restart(bool enable = true) -> builder&;

      private:
         [[nodiscard]] auto create_pipeline(vk::UniquePipelineLayout layout) const
            -> vkn::result<graphics_pipeline>;

      private:
         util::logger* const m_plogger;

         struct info
         {
            vk::Device device;
            vk::RenderPass render_pass;

            shader_dynamic_array<const vkn::shader*> shaders;

            vk::PrimitiveTopology topology{vk::PrimitiveTopology::eTriangleList};
            bool enable_primitive_restart{false};

            util::small_dynamic_array<vk::Viewport, 1> viewports;
            util::small_dynamic_array<vk::Rect2D, 1> scissors;
         } m_info;
      };

   private:
      struct create_info
      {
         vk::UniquePipeline pipeline{nullptr};
         vk::UniquePipelineLayout pipeline_layout{nullptr};
      };

      graphics_pipeline(create_info&& info) noexcept;

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
      /**
       * Turn an error flag and a standard error code into a vkn::error
       */
      inline static auto make_error(error err, std::error_code ec) -> vkn::error
      {
         return {{static_cast<int>(err), m_category}, static_cast<vk::Result>(ec.value())};
      }
   };
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::graphics_pipeline::error> : true_type
   {
   };
} // namespace std
