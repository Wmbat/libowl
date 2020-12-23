#include <fluid_simulation/render/pipeline.hpp>

#include <monads/try.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>

#include <cassert>
#include <span>
#include <utility>

namespace detail
{
   auto to_shader_stage_flag(vkn::shader_type stage) noexcept -> vk::ShaderStageFlagBits
   {
      switch (stage)
      {
         case vkn::shader_type::vertex:
            return vk::ShaderStageFlagBits::eVertex;
         case vkn::shader_type::fragment:
            return vk::ShaderStageFlagBits::eFragment;
         case vkn::shader_type::compute:
            return vk::ShaderStageFlagBits::eCompute;
         case vkn::shader_type::geometry:
            return vk::ShaderStageFlagBits::eGeometry;
         case vkn::shader_type::tess_eval:
            return vk::ShaderStageFlagBits::eTessellationEvaluation;
         case vkn::shader_type::tess_control:
            return vk::ShaderStageFlagBits::eTessellationControl;
         default:
            return {};
      }
   }
   auto
   check_vertex_attribute_support(const vkn::shader* p_shader,
                                  std::span<const vk::VertexInputAttributeDescription> attributes,
                                  util::logger_wrapper logger) -> bool
   {
      const auto& data = p_shader->get_data();
      for (const auto& attrib : attributes)
      {
         bool is_attrib_supported = false;
         for (const auto& input : data.inputs)
         {
            if (attrib.location == input.value())
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

graphics_pipeline::graphics_pipeline(graphics_pipeline_create_info&& info) :
   m_set_layouts{create_descriptor_set_layouts(info.device, info.shader_infos, info.logger)},
   m_push_constants{populate_push_constants(info.shader_infos)}, mp_render_pass{&info.pass}
{
   m_pipeline_layout = create_pipeline_layout(info.device);
   m_pipeline = create_pipeline(info.device, info.shader_infos, info.bindings, info.attributes,
                                info.viewports, info.scissors, info.logger);

   std::string msg = "Graphics pipeline created with:";

   msg += "\n\tset layouts = { ";
   for (std::size_t i = 0; const auto& [name, layout] : m_set_layouts)
   {
      msg += name;

      if (i++ != 0)
      {
         msg += ",";
      }
   }
   msg += " }";

   msg += "\n\tpush constants = { ";
   for (std::size_t i = 0; const auto& [name, push] : m_push_constants)
   {
      msg += name;

      if (i++ != 0)
      {
         msg += ",";
      }
   }
   msg += " }";

   info.logger.info(msg);
}

auto graphics_pipeline::value() const noexcept -> vk::Pipeline
{
   return m_pipeline.get();
}
auto graphics_pipeline::layout() const noexcept -> vk::PipelineLayout
{
   return m_pipeline_layout.get();
}
auto graphics_pipeline::get_descriptor_set_layout(const std::string& name) const
   -> const vkn::descriptor_set_layout&
{
   return m_set_layouts.at(name);
}
auto graphics_pipeline::get_push_constant_ranges(const std::string& name) const
   -> const vk::PushConstantRange&
{
   return m_push_constants.at(name);
}

auto graphics_pipeline::populate_push_constants(std::span<const pipeline_shader_data> shader_infos)
   -> push_constant_map
{
   push_constant_map ranges;

   for (const auto& info : shader_infos)
   {
      for (const auto& push : info.push_constants)
      {
         ranges.insert_or_assign(
            push.name,
            vk::PushConstantRange{.stageFlags = to_shader_flag(info.p_shader->stage()),
                                  .offset = static_cast<uint32_t>(push.offset.value()),
                                  .size = static_cast<uint32_t>(push.size.value())});
      }
   }

   return ranges;
}

auto graphics_pipeline::create_descriptor_set_layouts(
   const vkn::device& device, std::span<const pipeline_shader_data> shader_infos,
   util::logger_wrapper logger) -> set_layout_map
{
   set_layout_map set_layouts;

   for (const auto& shader_info : shader_infos)
   {
      for (const auto& set_info : shader_info.set_layouts)
      {
         auto bindings =
            set_info.bindings | ranges::views::transform([&](set_layout_binding binding) {
               vk::DescriptorSetLayoutBinding result;
               result.binding = static_cast<std::uint32_t>(binding.binding.value());
               result.descriptorType = binding.descriptor_type;
               result.descriptorCount = binding.descriptor_count.value();
               result.stageFlags = detail::to_shader_stage_flag(shader_info.p_shader->stage());

               return result;
            });

         set_layouts.insert_or_assign(
            set_info.name,
            vkn::descriptor_set_layout{{.device = device,
                                        .bindings = bindings | ranges::to<crl::dynamic_array>,
                                        .logger = logger}});
      }
   }

   return set_layouts;
}

auto graphics_pipeline::create_pipeline_layout(const vkn::device& device)
   -> vk::UniquePipelineLayout
{
   crl::dynamic_array<vk::DescriptorSetLayout> layouts;
   layouts.reserve(std::size(m_set_layouts));

   for (const auto& [name, layout] : m_set_layouts)
   {
      layouts.append(layout.value());
   }

   crl::dynamic_array<vk::PushConstantRange> push_constants;
   push_constants.reserve(std::size(m_push_constants));

   for (const auto& [name, push] : m_push_constants)
   {
      push_constants.append(push);
   }

   return device.logical().createPipelineLayoutUnique(
      {.pNext = nullptr,
       .flags = {},
       .setLayoutCount = static_cast<std::uint32_t>(std::size(layouts)),
       .pSetLayouts = std::data(layouts),
       .pushConstantRangeCount = static_cast<std::uint32_t>(std::size(push_constants)),
       .pPushConstantRanges = std::data(push_constants)});
}

auto graphics_pipeline::create_pipeline(const vkn::device& device,
                                        std::span<const pipeline_shader_data> shader_infos,
                                        std::span<vk::VertexInputBindingDescription> bindings,
                                        std::span<vk::VertexInputAttributeDescription> attributes,
                                        std::span<vk::Viewport> viewports,
                                        std::span<vk::Rect2D> scissors, util::logger_wrapper logger)
   -> vk::UniquePipeline
{
   crl::dynamic_array<vk::PipelineShaderStageCreateInfo> shader_stage_info{};
   shader_stage_info.reserve(std::size(shader_infos));

   util::index_t vertex_shader_index{std::numeric_limits<std::size_t>::max()};
   for (std::uint32_t index = 0; const auto& info : shader_infos)
   {
      shader_stage_info.append(vk::PipelineShaderStageCreateInfo{}
                                  .setPNext(nullptr)
                                  .setFlags({})
                                  .setPSpecializationInfo(nullptr)
                                  .setStage(detail::to_shader_stage_flag(info.p_shader->stage()))
                                  .setModule(info.p_shader->value())
                                  .setPName("main"));

      if (info.p_shader->stage() == vkn::shader_type::vertex)
      {
         vertex_shader_index = index;
      }

      ++index;
   }

   const auto* p_vertex_shader = shader_infos[vertex_shader_index.value()].p_shader;
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

   const auto rasterization_state_create_info = vk::PipelineRasterizationStateCreateInfo{}
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

   const auto colour_blend_state_create_info = vk::PipelineColorBlendStateCreateInfo{}
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
                        .setLayout(m_pipeline_layout.get())
                        .setRenderPass(mp_render_pass->value())
                        .setSubpass(0)
                        .setBasePipelineHandle(nullptr);

   return device.logical().createGraphicsPipelineUnique(nullptr, info);
}

/**
 * A struct used for error handling and displaying error messages
 */
struct graphics_pipeline_error_category : std::error_category
{
   /**
    * The name of the vkn object the error appeared from.
    */
   [[nodiscard]] auto name() const noexcept -> const char* override
   {
      return "vkn_graphics_pipeline";
   } // namespace vkn
   /**
    * Get the message associated with a specific error code.
    */
   [[nodiscard]] auto message(int err) const -> std::string override
   {
      return to_string(static_cast<graphics_pipeline_error>(err));
   }
};

inline static const graphics_pipeline_error_category graphics_pipeline_category{};

auto to_string(graphics_pipeline_error err) -> std::string
{
   using err_t = graphics_pipeline_error;

   switch (err)
   {
      case err_t::failed_to_create_descriptor_set_layout:
         return "failed_to_create_descriptor_set_layout";
      case err_t::failed_to_create_pipeline:
         return "failed_to_create_pipeline";
      case err_t::invalid_vertex_shader_bindings:
         return "invalid_vertex_shader_bindings";
      case err_t::failed_to_create_pipeline_layout:
         return "failed_to_create_pipeline_layout";
      default:
         return "UNKNOWN";
   }
}
auto to_err_code(graphics_pipeline_error err) -> util::error_t
{
   return {{static_cast<int>(err), graphics_pipeline_category}};
}
