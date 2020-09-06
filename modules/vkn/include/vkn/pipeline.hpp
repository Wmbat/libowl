#pragma once

#include <vkn/core.hpp>
#include <vkn/device.hpp>
#include <vkn/render_pass.hpp>
#include <vkn/shader.hpp>

namespace vkn
{
   enum struct graphics_pipeline_error
   {
      failed_to_create_descriptor_set_layout,
      failed_to_create_pipeline_layout,
      failed_to_create_pipeline
   };

   auto to_string(graphics_pipeline_error err) -> std::string;
   auto make_error(graphics_pipeline_error err, std::error_code ec) -> vkn::error;

   class graphics_pipeline final : public owning_handle<vk::Pipeline>
   {
      static constexpr std::size_t expected_shader_count{2u};

   public:
      [[nodiscard]] auto layout() const noexcept -> vk::PipelineLayout;
      [[nodiscard]] auto device() const noexcept -> vk::Device;

   private:
      vk::UniquePipelineLayout m_pipeline_layout{nullptr};

   public:
      class builder final
      {
         template <typename any_>
         using shader_dynamic_array = util::small_dynamic_array<any_, expected_shader_count>;

      public:
         builder(const vkn::device& device, const vkn::render_pass& render_pass,
                 std::shared_ptr<util::logger> p_logger);

         auto build() -> vkn::result<graphics_pipeline>;

         auto add_shader(const vkn::shader& shader) noexcept -> builder&;
         auto add_viewport(const vk::Viewport& viewport, const vk::Rect2D& scissor) noexcept
            -> builder&;
         auto add_vertex_binding(vk::VertexInputBindingDescription&& binding) noexcept -> builder&;
         auto add_vertex_attribute(vk::VertexInputAttributeDescription&& attribute) noexcept
            -> builder&;
         auto add_set_layout(const util::dynamic_array<vk::DescriptorSetLayoutBinding>& binding)
            -> builder&;

      private:
         [[nodiscard]] auto create_pipeline(vk::UniquePipelineLayout layout) const
            -> vkn::result<graphics_pipeline>;

      private:
         std::shared_ptr<util::logger> mp_logger;

         struct descriptor_set_layout_info
         {
            util::dynamic_array<vk::DescriptorSetLayoutBinding> binding;
         };

         struct info
         {
            vk::Device device;
            vk::RenderPass render_pass;

            shader_dynamic_array<const vkn::shader*> shaders;

            util::small_dynamic_array<vk::Viewport, 1> viewports;
            util::small_dynamic_array<vk::Rect2D, 1> scissors;

            util::dynamic_array<vk::VertexInputBindingDescription> binding_descriptions;
            util::dynamic_array<vk::VertexInputAttributeDescription> attribute_descriptions;

            util::dynamic_array<descriptor_set_layout_info> set_layouts;
         } m_info;
      };
   };
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::graphics_pipeline_error> : true_type
   {
   };
} // namespace std
