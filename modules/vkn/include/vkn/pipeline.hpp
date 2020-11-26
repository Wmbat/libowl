#pragma once

#include <vkn/core.hpp>
#include <vkn/descriptor_set_layout.hpp>
#include <vkn/device.hpp>
#include <vkn/render_pass.hpp>
#include <vkn/shader.hpp>

#include <util/error.hpp>
#include <util/strong_type.hpp>

namespace vkn
{
   using vertex_bindings_array = util::dynamic_array<vk::VertexInputBindingDescription>;
   using vertex_attributes_array = util::dynamic_array<vk::VertexInputAttributeDescription>;

   enum struct pipeline_type
   {
      graphics,
      compute
   };

   enum struct graphics_pipeline_error
   {
      failed_to_create_descriptor_set_layout,
      failed_to_create_pipeline_layout,
      invalid_vertex_shader_bindings,
      failed_to_create_pipeline
   };

   struct set_layout_binding
   {
      util::index_t binding{};
      vk::DescriptorType descriptor_type{};
      util::count32_t descriptor_count{};
   };

   struct set_layout_data
   {
      std::string name{};
      util::dynamic_array<set_layout_binding> bindings{};
   };

   struct push_constant_data
   {
      std::string name{};
      util::size_t size{};
      util::size_t offset{};
   };

   struct pipeline_shader_data
   {
      vkn::shader* p_shader{nullptr};
      util::dynamic_array<set_layout_data> set_layouts{};
      util::dynamic_array<push_constant_data> push_constants{};
   };

   class graphics_pipeline final
   {
      static constexpr std::size_t expected_shader_count{2U};

   public:
      struct create_info
      {
         const vkn::device& device;
         const vkn::render_pass& render_pass;

         util::logger_wrapper logger{};

         vertex_bindings_array bindings{};
         vertex_attributes_array attributes{};

         util::dynamic_array<vk::Viewport> viewports{};
         util::dynamic_array<vk::Rect2D> scissors{};

         util::dynamic_array<pipeline_shader_data> shader_infos{};
      };

      static auto make(create_info&& info) -> util::result<graphics_pipeline>;

   public:
      [[nodiscard]] auto value() const noexcept -> vk::Pipeline;
      [[nodiscard]] auto layout() const noexcept -> vk::PipelineLayout;

      auto get_descriptor_set_layout(const std::string& name) const
         -> const vkn::descriptor_set_layout&;
      auto get_push_constant_ranges(const std::string& name) const -> const vk::PushConstantRange&;

   private:
      vk::UniquePipeline m_pipeline{nullptr};
      vk::UniquePipelineLayout m_pipeline_layout{nullptr};

      std::unordered_map<std::string, vkn::descriptor_set_layout> m_set_layouts{};
      std::unordered_map<std::string, vk::PushConstantRange> m_push_constants{};
   };

   auto to_string(graphics_pipeline_error err) -> std::string;
   auto to_err_code(graphics_pipeline_error err) -> util::error_t;
} // namespace vkn

namespace std
{
   template <>
   struct is_error_code_enum<vkn::graphics_pipeline_error> : true_type
   {
   };
} // namespace std
