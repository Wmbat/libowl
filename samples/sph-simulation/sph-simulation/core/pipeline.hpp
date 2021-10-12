#pragma once

#include <sph-simulation/core.hpp>
#include <sph-simulation/render/core/render_pass.hpp>

#include <libcacao/descriptor_set_layout.hpp>
#include <libcacao/device.hpp>
#include <libcacao/shader.hpp>

#include <span>

using vertex_bindings_array = std::vector<vk::VertexInputBindingDescription>;
using vertex_attributes_array = std::vector<vk::VertexInputAttributeDescription>;

enum struct pipeline_type
{
   graphics,
   compute,
   none
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

   mannele::log_ptr logger{};

   vertex_bindings_array bindings{};
   vertex_attributes_array attributes{};

   std::vector<vk::Viewport> viewports{};
   std::vector<vk::Rect2D> scissors{};

   std::vector<pipeline_shader_data> shader_infos{};
};

struct compute_pipeline_create_info
{
   const cacao::device& device;

   pipeline_shader_data shader_info{};

   mannele::log_ptr logger{};
};

namespace detail
{
   using set_layout_map = std::unordered_map<std::string, cacao::descriptor_set_layout>;
   using push_constant_map = std::unordered_map<std::string, vk::PushConstantRange>;

   class pipeline_base
   {
      using set_layout_map = detail::set_layout_map;
      using push_constant_map = detail::push_constant_map;

   public:
      pipeline_base(const cacao::device& device, std::span<const pipeline_shader_data> shader_infos,
                    mannele::log_ptr logger);
      pipeline_base(const cacao::device& device, const pipeline_shader_data& shader_infos,
                    mannele::log_ptr logger);

      [[nodiscard]] auto layout() const noexcept -> vk::PipelineLayout;

      auto get_descriptor_set_layout(const std::string& name) const
         -> const cacao::descriptor_set_layout&;
      auto get_push_constant_ranges(const std::string& name) const -> const vk::PushConstantRange&;

   private:
      set_layout_map m_set_layouts{};
      push_constant_map m_push_constants{};

      vk::UniquePipelineLayout m_pipeline_layout{nullptr};
   };

   auto create_graphics_pipeline(const cacao::device& device, const render_pass& render_pass,
                                 vk::PipelineLayout layout,
                                 std::span<const pipeline_shader_data> shader_infos,
                                 std::span<vk::VertexInputBindingDescription> bindings,
                                 std::span<vk::VertexInputAttributeDescription> attributes,
                                 std::span<vk::Viewport> viewports, std::span<vk::Rect2D> scissors,
                                 mannele::log_ptr logger) -> vk::UniquePipeline;

   auto create_compute_pipeline(const cacao::device& device, vk::PipelineLayout layout,
                                const pipeline_shader_data& shader_info)
      -> vk::UniquePipeline;
} // namespace detail

template <pipeline_type Type>
class pipeline : public detail::pipeline_base
{
   using base = detail::pipeline_base;

public:
   pipeline() = default;
   explicit pipeline(graphics_pipeline_create_info&& info) requires(Type ==
                                                                    pipeline_type::graphics) :
      base(info.device, info.shader_infos, info.logger),
      m_pipeline(detail::create_graphics_pipeline(info.device, info.pass, base::layout(),
                                                  info.shader_infos, info.bindings, info.attributes,
                                                  info.viewports, info.scissors, info.logger))
   {
      info.logger.debug("graphics pipeline created");
   }
   explicit pipeline(compute_pipeline_create_info&& info) requires(Type == pipeline_type::compute) :
      base(info.device, info.shader_info, info.logger),
      m_pipeline(detail::create_compute_pipeline(info.device, base::layout(), info.shader_info))
   {
      info.logger.debug("compute pipeline created");
   }

   [[nodiscard]] auto value() const noexcept -> vk::Pipeline { return m_pipeline.get(); }

private:
   vk::UniquePipeline m_pipeline{nullptr};
};
