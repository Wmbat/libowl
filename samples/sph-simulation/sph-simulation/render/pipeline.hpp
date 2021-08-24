#pragma once

#include <sph-simulation/core.hpp>
#include <sph-simulation/render/render_pass.hpp>

#include <libcacao/descriptor_set_layout.hpp>
#include <libcacao/shader.hpp>

using vertex_bindings_array = std::vector<vk::VertexInputBindingDescription>;
using vertex_attributes_array = std::vector<vk::VertexInputAttributeDescription>;

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
   mannele::u32 binding{};
   vk::DescriptorType descriptor_type{};
   mannele::u32 descriptor_count{};
};

struct set_layout_data
{
   std::string name{};
   std::vector<set_layout_binding> bindings{};
};

struct push_constant_data
{
   std::string name{};
   mannele::u64 size{};
   mannele::u64 offset{};
};

struct pipeline_shader_data
{
   cacao::shader* p_shader{nullptr};
   std::vector<set_layout_data> set_layouts{};
   std::vector<push_constant_data> push_constants{};
};

struct graphics_pipeline_create_info
{
   const cacao::device& device;
   const render_pass& pass;

   util::log_ptr logger{};

   vertex_bindings_array bindings{};
   vertex_attributes_array attributes{};

   std::vector<vk::Viewport> viewports{};
   std::vector<vk::Rect2D> scissors{};

   std::vector<pipeline_shader_data> shader_infos{};
};

class graphics_pipeline final
{
   static constexpr std::size_t expected_shader_count{2U};

   using set_layout_map = std::unordered_map<std::string, cacao::descriptor_set_layout>;
   using push_constant_map = std::unordered_map<std::string, vk::PushConstantRange>;

public:
   graphics_pipeline() = default;
   graphics_pipeline(graphics_pipeline_create_info&& info);

   [[nodiscard]] auto value() const noexcept -> vk::Pipeline;
   [[nodiscard]] auto layout() const noexcept -> vk::PipelineLayout;

   auto get_descriptor_set_layout(const std::string& name) const
      -> const cacao::descriptor_set_layout&;
   auto get_push_constant_ranges(const std::string& name) const -> const vk::PushConstantRange&;

private:
   auto populate_push_constants(std::span<const pipeline_shader_data> shader_infos)
      -> push_constant_map;
   auto create_descriptor_set_layouts(const cacao::device& device,
                                      std::span<const pipeline_shader_data> shader_infos,
                                      util::log_ptr logger) -> set_layout_map;
   auto create_pipeline_layout(const cacao::device& device) -> vk::UniquePipelineLayout;
   auto create_pipeline(const cacao::device& device,
                        std::span<const pipeline_shader_data> shader_infos,
                        std::span<vk::VertexInputBindingDescription> bindings,
                        std::span<vk::VertexInputAttributeDescription> attributes,
                        std::span<vk::Viewport> viewports, std::span<vk::Rect2D> scissors,
                        util::log_ptr logger) -> vk::UniquePipeline;

private:
   vk::UniquePipeline m_pipeline{nullptr};
   vk::UniquePipelineLayout m_pipeline_layout{nullptr};

   set_layout_map m_set_layouts{};
   push_constant_map m_push_constants{};

   const render_pass* mp_render_pass{nullptr};
};

auto make_error_condition(graphics_pipeline_error err) -> std::error_condition;

namespace std
{
   template <>
   struct is_error_code_enum<graphics_pipeline_error> : true_type
   {
   };
} // namespace std
