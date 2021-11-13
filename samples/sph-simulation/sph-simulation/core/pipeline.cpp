#include <sph-simulation/core/pipeline.hpp>

#include <libreglisse/try.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view.hpp>
#include <range/v3/view/transform.hpp>

#include <cassert>
#include <span>
#include <utility>

namespace rv = ranges::views;

namespace detail
{
   auto to_shader_stage_flag(cacao::shader_type stage) noexcept -> vk::ShaderStageFlagBits
   {
      switch (stage)
      {
         case cacao::shader_type::vertex:
            return vk::ShaderStageFlagBits::eVertex;
         case cacao::shader_type::fragment:
            return vk::ShaderStageFlagBits::eFragment;
         case cacao::shader_type::compute:
            return vk::ShaderStageFlagBits::eCompute;
         case cacao::shader_type::geometry:
            return vk::ShaderStageFlagBits::eGeometry;
         case cacao::shader_type::tess_eval:
            return vk::ShaderStageFlagBits::eTessellationEvaluation;
         case cacao::shader_type::tess_control:
            return vk::ShaderStageFlagBits::eTessellationControl;
         default:
            return {};
      }
   }
   auto
   check_vertex_attribute_support(const cacao::shader* p_shader,
                                  std::span<const vk::VertexInputAttributeDescription> attributes,
                                  mannele::log_ptr logger) -> bool
   {
      std::span inputs = p_shader->input_ids();
      for (const auto& attrib : attributes)
      {
         bool is_attrib_supported = false;
         for (const auto& input : inputs)
         {
            if (attrib.location == input)
            {
               is_attrib_supported = true;
            }
         }

         if (!is_attrib_supported)
         {
            logger.error("[vulkan] vertex shader attribute at location {} and binding {} is not "
                         "supported by the shader",
                         attrib.location, attrib.binding);

            return false;
         }
      }

      return true;
   }

}; // namespace detail

namespace detail
{
   auto create_descriptor_set_layouts(const cacao::device& device,
                                      std::span<const pipeline_shader_data> shader_infos,
                                      mannele::log_ptr logger)
      -> std::unordered_map<std::string, cacao::descriptor_set_layout>
   {
      std::unordered_map<std::string, cacao::descriptor_set_layout> set_layouts;

      for (const auto& shader_info : shader_infos)
      {
         for (const auto& set_info : shader_info.set_layouts)
         {
            auto bindings =
               set_info.bindings | ranges::views::transform([&](set_layout_binding binding) {
                  vk::DescriptorSetLayoutBinding result;
                  result.binding = static_cast<std::uint32_t>(binding.binding);
                  result.descriptorType = binding.descriptor_type;
                  result.descriptorCount = binding.descriptor_count;
                  result.stageFlags = detail::to_shader_stage_flag(shader_info.p_shader->type());

                  return result;
               });

            set_layouts.insert_or_assign(
               set_info.name,
               cacao::descriptor_set_layout{{.device = device,
                                             .bindings = bindings | ranges::to<std::vector>,
                                             .logger = logger}});
         }
      }

      return set_layouts;
   }

   auto populate_push_constants(std::span<const pipeline_shader_data> shader_infos)
      -> std::unordered_map<std::string, vk::PushConstantRange>
   {
      std::unordered_map<std::string, vk::PushConstantRange> ranges;

      for (const auto& info : shader_infos)
      {
         for (const auto& push : info.push_constants)
         {
            ranges.insert_or_assign(
               push.name,
               vk::PushConstantRange{.stageFlags = cacao::to_shader_flag(info.p_shader->type()),
                                     .offset = static_cast<uint32_t>(push.offset),
                                     .size = static_cast<uint32_t>(push.size)});
         }
      }

      return ranges;
   }

   auto create_pipeline_layout(const cacao::device& device, const set_layout_map& raw_set_layouts,
                               const push_constant_map& raw_push_constants)
      -> vk::UniquePipelineLayout
   {
      // clang-format off
      
      std::vector layouts = raw_set_layouts 
         | rv::values 
         | rv::transform([](const auto& layout) { return layout.value(); }) 
         | ranges::to<std::vector>;

      std::vector<vk::PushConstantRange> push_constants;
      for(const auto& test : raw_push_constants | rv::values)
      {
         push_constants.push_back(test);
      }

      // clang-format on

      return device.logical().createPipelineLayoutUnique(
         {.pNext = nullptr,
          .flags = {},
          .setLayoutCount = static_cast<std::uint32_t>(std::size(layouts)),
          .pSetLayouts = std::data(layouts),
          .pushConstantRangeCount = static_cast<std::uint32_t>(std::size(push_constants)),
          .pPushConstantRanges = std::data(push_constants)});
   }

   auto create_graphics_pipeline(const cacao::device& device, const render_pass& render_pass,
                                 vk::PipelineLayout layout,
                                 std::span<const pipeline_shader_data> shader_infos,
                                 std::span<vk::VertexInputBindingDescription> bindings,
                                 std::span<vk::VertexInputAttributeDescription> attributes,
                                 std::span<vk::Viewport> viewports, std::span<vk::Rect2D> scissors,
                                 mannele::log_ptr logger) -> vk::UniquePipeline
   {
      const auto logical = device.logical();

      std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_info{};
      shader_stage_info.reserve(std::size(shader_infos));

      mannele::u64 vertex_shader_index{std::numeric_limits<std::size_t>::max()};
      for (std::uint32_t index = 0; const auto& info : shader_infos)
      {
         shader_stage_info.push_back(
            vk::PipelineShaderStageCreateInfo{}
               .setPNext(nullptr)
               .setFlags({})
               .setPSpecializationInfo(nullptr)
               .setStage(detail::to_shader_stage_flag(info.p_shader->type()))
               .setModule(info.p_shader->module())
               .setPName("main"));

         if (info.p_shader->type() == cacao::shader_type::vertex)
         {
            vertex_shader_index = index;
         }

         ++index;
      }

      const auto* p_vertex_shader = shader_infos[vertex_shader_index].p_shader;
      if (!detail::check_vertex_attribute_support(p_vertex_shader, attributes, logger))
      {
         // TODO: ERROR
         // return monad::err(to_err_code(graphics_pipeline_error::invalid_vertex_shader_bindings));
      }

      const auto vertex_input_state_create_info =
         vk::PipelineVertexInputStateCreateInfo{}
            .setVertexBindingDescriptionCount(static_cast<std::uint32_t>(std::size(bindings)))
            .setPVertexBindingDescriptions(std::data(bindings))
            .setVertexAttributeDescriptionCount(static_cast<std::uint32_t>(std::size(attributes)))
            .setPVertexAttributeDescriptions(std::data(attributes));

      const auto input_assembly_state_create_info =
         vk::PipelineInputAssemblyStateCreateInfo{}
            .setTopology(vk::PrimitiveTopology::eTriangleList)
            .setPrimitiveRestartEnable(false);

      const auto viewport_state_create_info =
         vk::PipelineViewportStateCreateInfo{}
            .setViewportCount(static_cast<std::uint32_t>(std::size(viewports)))
            .setPViewports(viewports.data())
            .setScissorCount(static_cast<std::uint32_t>(std::size(scissors)))
            .setPScissors(scissors.data());

      const auto rasterization_state_create_info =
         vk::PipelineRasterizationStateCreateInfo{}
            .setDepthClampEnable(false)
            .setRasterizerDiscardEnable(false)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setLineWidth(1.0F)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eCounterClockwise)
            .setDepthBiasEnable(true);

      const auto multisample_state_create_info =
         vk::PipelineMultisampleStateCreateInfo{}
            .setSampleShadingEnable(false)
            .setRasterizationSamples(vk::SampleCountFlagBits::e1);

      const vk::PipelineDepthStencilStateCreateInfo depth_stencil_create_info{
         .depthTestEnable = VK_TRUE,
         .depthWriteEnable = VK_TRUE,
         .depthCompareOp = vk::CompareOp::eLess,
         .depthBoundsTestEnable = VK_FALSE,
         .stencilTestEnable = VK_FALSE};

      const auto colour_blend_attachment_state =
         vk::PipelineColorBlendAttachmentState{}
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                               vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
            .setBlendEnable(false);

      const auto colour_blend_state_create_info =
         vk::PipelineColorBlendStateCreateInfo{}
            .setLogicOpEnable(false)
            .setLogicOp(vk::LogicOp::eCopy)
            .setAttachmentCount(1U)
            .setPAttachments(&colour_blend_attachment_state)
            .setBlendConstants({0.0F, 0.0F, 0.0F, 0.0F});

      const auto info = vk::GraphicsPipelineCreateInfo{}
                           .setPNext(nullptr)
                           .setFlags({})
                           .setStageCount(static_cast<std::uint32_t>(std::size(shader_stage_info)))
                           .setPStages(shader_stage_info.data())
                           .setPVertexInputState(&vertex_input_state_create_info)
                           .setPInputAssemblyState(&input_assembly_state_create_info)
                           .setPRasterizationState(&rasterization_state_create_info)
                           .setPViewportState(&viewport_state_create_info)
                           .setPMultisampleState(&multisample_state_create_info)
                           .setPColorBlendState(&colour_blend_state_create_info)
                           .setPDepthStencilState(&depth_stencil_create_info)
                           .setLayout(layout)
                           .setRenderPass(render_pass.value())
                           .setSubpass(0)
                           .setBasePipelineHandle(nullptr);

      return logical.createGraphicsPipelineUnique(nullptr, info).value;
   }

   auto create_compute_pipeline(const cacao::device& device, vk::PipelineLayout layout,
                                const pipeline_shader_data& shader_info) -> vk::UniquePipeline
   {
      const auto logical = device.logical();

      const auto info = vk::ComputePipelineCreateInfo{
         .flags = {},
         .stage = {.pNext = nullptr,
                   .flags = {},
                   .stage = detail::to_shader_stage_flag(shader_info.p_shader->type()),
                   .module = shader_info.p_shader->module(),
                   .pName = "main",
                   .pSpecializationInfo = nullptr},
         .layout = layout,
         .basePipelineHandle = nullptr};

      return logical.createComputePipelineUnique(nullptr, info).value;
   }

} // namespace detail

///////////////////////////////////////////////

detail::pipeline_base::pipeline_base(const cacao::device& device,
                                     std::span<const pipeline_shader_data> shader_infos,
                                     mannele::log_ptr logger) :

   m_set_layouts(detail::create_descriptor_set_layouts(device, shader_infos, logger)),
   m_push_constants(detail::populate_push_constants(shader_infos)),
   m_pipeline_layout(detail::create_pipeline_layout(device, m_set_layouts, m_push_constants))
{}
detail::pipeline_base::pipeline_base(const cacao::device& device,
                                     const pipeline_shader_data& shader_infos,
                                     mannele::log_ptr logger)
{}

[[nodiscard]] auto detail::pipeline_base::layout() const noexcept -> vk::PipelineLayout
{
   return m_pipeline_layout.get();
}

auto detail::pipeline_base::get_descriptor_set_layout(const std::string& name) const
   -> const cacao::descriptor_set_layout&
{
   return m_set_layouts.at(name);
}
auto detail::pipeline_base::get_push_constant_ranges(const std::string& name) const
   -> const vk::PushConstantRange&
{
   return m_push_constants.at(name);
}

///////////////////////////////////////////////
