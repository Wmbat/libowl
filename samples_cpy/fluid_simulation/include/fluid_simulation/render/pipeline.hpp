#pragma once

#include <fluid_simulation/core.hpp>
#include <fluid_simulation/render/render_pass.hpp>

#include <cacao/util/error.hpp>
#include <cacao/util/strong_type.hpp>
#include <cacao/vulkan/core.hpp>
#include <cacao/vulkan/descriptor_set_layout.hpp>
#include <cacao/vulkan/shader.hpp>

using vertex_bindings_array = crl::dynamic_array<vk::VertexInputBindingDescription>;
using vertex_attributes_array = crl::dynamic_array<vk::VertexInputAttributeDescription>;

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
   cacao::index_t binding{};
   vk::DescriptorType descriptor_type{};
   cacao::count32_t descriptor_count{};
};

struct set_layout_data
{
   std::string name{};
   crl::dynamic_array<set_layout_binding> bindings{};
};

struct push_constant_data
{
   std::string name{};
   cacao::size_t size{};
   cacao::size_t offset{};
};

struct pipeline_shader_data
{
   vkn::shader* p_shader{nullptr};
   crl::dynamic_array<set_layout_data> set_layouts{};
   crl::dynamic_array<push_constant_data> push_constants{};
};

struct graphics_pipeline_create_info
{
   const cacao::device& device;
   const render_pass& pass;

   util::logger_wrapper logger{};

   vertex_bindings_array bindings{};
   vertex_attributes_array attributes{};

   crl::dynamic_array<vk::Viewport> viewports{};
   crl::dynamic_array<vk::Rect2D> scissors{};

   crl::dynamic_array<pipeline_shader_data> shader_infos{};
};

class graphics_pipeline final
{
   static constexpr std::size_t expected_shader_count{2U};

   using set_layout_map = std::unordered_map<std::string, vkn::descriptor_set_layout>;
   using push_constant_map = std::unordered_map<std::string, vk::PushConstantRange>;

public:
   graphics_pipeline() = default;
   graphics_pipeline(graphics_pipeline_create_info&& info);

   [[nodiscard]] auto value() const noexcept -> vk::Pipeline;
   [[nodiscard]] auto layout() const noexcept -> vk::PipelineLayout;

   auto get_descriptor_set_layout(const std::string& name) const
      -> const vkn::descriptor_set_layout&;
   auto get_push_constant_ranges(const std::string& name) const -> const vk::PushConstantRange&;

private:
   auto populate_push_constants(std::span<const pipeline_shader_data> shader_infos)
      -> push_constant_map;
   auto create_descriptor_set_layouts(const cacao::device& device,
                                      std::span<const pipeline_shader_data> shader_infos,
                                      util::logger_wrapper logger) -> set_layout_map;
   auto create_pipeline_layout(const cacao::device& device) -> vk::UniquePipelineLayout;
   auto create_pipeline(const cacao::device& device,
                        std::span<const pipeline_shader_data> shader_infos,
                        std::span<vk::VertexInputBindingDescription> bindings,
                        std::span<vk::VertexInputAttributeDescription> attributes,
                        std::span<vk::Viewport> viewports, std::span<vk::Rect2D> scissors,
                        util::logger_wrapper logger) -> vk::UniquePipeline;

private:
   vk::UniquePipeline m_pipeline{nullptr};
   vk::UniquePipelineLayout m_pipeline_layout{nullptr};

   set_layout_map m_set_layouts{};
   push_constant_map m_push_constants{};

   const render_pass* mp_render_pass{nullptr};
};

auto to_string(graphics_pipeline_error err) -> std::string;
auto to_err_code(graphics_pipeline_error err) -> util::error_t;

namespace std
{
   template <>
   struct is_error_code_enum<graphics_pipeline_error> : true_type
   {
   };
} // namespace std
