#include "monads/try.hpp"
#include <vkn/pipeline.hpp>

#include <cassert>

namespace vkn
{
   namespace detail
   {
      auto to_shader_stage_flag(vkn::shader::type stage) noexcept -> vk::ShaderStageFlagBits
      {
         switch (stage)
         {
            case shader::type::vertex:
               return vk::ShaderStageFlagBits::eVertex;
            case shader::type::fragment:
               return vk::ShaderStageFlagBits::eFragment;
            case shader::type::compute:
               return vk::ShaderStageFlagBits::eCompute;
            case shader::type::geometry:
               return vk::ShaderStageFlagBits::eGeometry;
            case shader::type::tess_eval:
               return vk::ShaderStageFlagBits::eTessellationEvaluation;
            case shader::type::tess_control:
               return vk::ShaderStageFlagBits::eTessellationControl;
            default:
               break;
         }

         assert(false && "something went wrong here");
      }

      auto to_string(graphics_pipeline::error err) -> std::string
      {
         using err_t = vkn::graphics_pipeline::error;

         switch (err)
         {
            case err_t::failed_to_create_pipeline:
               return "failed_to_create_pipeline";
            case err_t::failed_to_create_pipeline_layout:
               return "failed_to_create_pipeline_layout";
            default:
               break;
         }

         assert(false && "something wrong happened");
      }
   }; // namespace detail

   auto graphics_pipeline::error_category::name() const noexcept -> const char*
   {
      return "vkn_graphics_pipeline";
   }
   auto graphics_pipeline::error_category::message(int err) const -> std::string
   {
      return detail::to_string(static_cast<graphics_pipeline::error>(err));
   }

   graphics_pipeline::graphics_pipeline(create_info&& info) noexcept :
      m_value{info.pipeline}, m_pipeline_layout{info.pipeline_layout}, m_device{info.device}
   {}
   graphics_pipeline::graphics_pipeline(graphics_pipeline&& other) noexcept
   {
      *this = std::move(other);
   }
   graphics_pipeline::~graphics_pipeline()
   {
      if (m_device)
      {
         if (m_value)
         {
            m_device.destroyPipeline(m_value);
         }

         if (m_pipeline_layout)
         {
            m_device.destroyPipelineLayout(m_pipeline_layout);
         }
      }
   }

   auto graphics_pipeline::operator=(graphics_pipeline&& rhs) noexcept -> graphics_pipeline&
   {
      std::swap(m_value, rhs.m_value);
      std::swap(m_pipeline_layout, rhs.m_pipeline_layout);
      std::swap(m_device, rhs.m_device);

      return *this;
   }

   auto graphics_pipeline::value() const noexcept -> value_type { return m_value; }
   auto graphics_pipeline::device() const noexcept -> vk::Device { return m_device; }
   auto graphics_pipeline::layout() const noexcept -> vk::PipelineLayout
   {
      return m_pipeline_layout;
   }

   graphics_pipeline::builder::builder(const vkn::device& device,
                                       const vkn::render_pass& render_pass, util::logger* plogger) :
      m_plogger{plogger}
   {
      m_info.device = device.value();
      m_info.render_pass = render_pass.value();
   }

   auto graphics_pipeline::builder::build() -> vkn::result<graphics_pipeline>
   {
      // Maybe move pipeline layout into it's own abstraction
      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createPipelineLayout({.pNext = nullptr,
                                                           .flags = {},
                                                           .setLayoutCount = 0u,
                                                           .pSetLayouts = nullptr,
                                                           .pushConstantRangeCount = 0u,
                                                           .pPushConstantRanges = nullptr});
             })
         .left_map([](vk::SystemError&& err) {
            return make_error(error::failed_to_create_pipeline_layout, err.code());
         })
         .right_flat_map([&](vk::PipelineLayout&& handle) {
            util::log_info(m_plogger, "[vkn] pipeline layout created");

            return create_pipeline(handle);
         });
   }

   auto graphics_pipeline::builder::add_shader(const vkn::shader& shader) noexcept -> builder&
   {
      m_info.shaders.emplace_back(&shader);
      return *this;
   }
   auto graphics_pipeline::builder::add_viewport(const vk::Viewport& viewport,
                                                 const vk::Rect2D& scissor) noexcept -> builder&
   {
      m_info.viewports.push_back(viewport);
      m_info.scissors.push_back(scissor);

      return *this;
   }

   auto graphics_pipeline::builder::set_topology(vk::PrimitiveTopology topology) noexcept
      -> builder&
   {
      m_info.topology = topology;
      return *this;
   }

   auto graphics_pipeline::builder::enable_primitive_restart(bool enable) -> builder&
   {
      m_info.enable_primitive_restart = enable;
      return *this;
   }

   auto graphics_pipeline::builder::create_pipeline(vk::PipelineLayout layout) const
      -> vkn::result<graphics_pipeline>
   {
      shader_dynamic_array<vk::PipelineShaderStageCreateInfo> shader_stage_info{};
      shader_stage_info.reserve(std::size(m_info.shaders));

      for (const auto* shader_handle : m_info.shaders)
      {
         util::log_info(m_plogger, R"([vkn] using shader "{0}" for graphics pipeline)",
                        shader_handle->name());

         shader_stage_info.emplace_back(
            vk::PipelineShaderStageCreateInfo{}
               .setPNext(nullptr)
               .setFlags({})
               .setPSpecializationInfo(nullptr)
               .setStage(detail::to_shader_stage_flag(shader_handle->stage()))
               .setModule(shader_handle->value())
               .setPName("main"));
      }

      const auto vertex_input_state_create_info = vk::PipelineVertexInputStateCreateInfo{}
                                                     .setVertexBindingDescriptionCount(0u)
                                                     .setPVertexBindingDescriptions(nullptr)
                                                     .setVertexAttributeDescriptionCount(0u)
                                                     .setPVertexAttributeDescriptions(nullptr);

      const auto input_assembly_state_create_info =
         vk::PipelineInputAssemblyStateCreateInfo{}
            .setTopology(m_info.topology)
            .setPrimitiveRestartEnable(m_info.enable_primitive_restart);

      const auto viewport_state_create_info = vk::PipelineViewportStateCreateInfo{}
                                                 .setViewportCount(std::size(m_info.viewports))
                                                 .setPViewports(m_info.viewports.data())
                                                 .setScissorCount(std::size(m_info.scissors))
                                                 .setPScissors(m_info.scissors.data());

      const auto rasterization_state_create_info = vk::PipelineRasterizationStateCreateInfo{}
                                                      .setDepthClampEnable(false)
                                                      .setRasterizerDiscardEnable(false)
                                                      .setPolygonMode(vk::PolygonMode::eFill)
                                                      .setLineWidth(1.0f)
                                                      .setCullMode(vk::CullModeFlagBits::eBack)
                                                      .setFrontFace(vk::FrontFace::eClockwise)
                                                      .setDepthBiasEnable(true);

      const auto multisample_state_create_info =
         vk::PipelineMultisampleStateCreateInfo{}
            .setSampleShadingEnable(false)
            .setRasterizationSamples(vk::SampleCountFlagBits::e1);

      const auto colour_blend_attachment_state =
         vk::PipelineColorBlendAttachmentState{}
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                               vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
            .setBlendEnable(false);

      const auto colour_blend_state_create_info =
         vk::PipelineColorBlendStateCreateInfo{}
            .setLogicOpEnable(false)
            .setLogicOp(vk::LogicOp::eCopy)
            .setAttachmentCount(1u)
            .setPAttachments(&colour_blend_attachment_state)
            .setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

      const auto create_info = vk::GraphicsPipelineCreateInfo{}
                                  .setPNext(nullptr)
                                  .setFlags({})
                                  .setStageCount(std::size(shader_stage_info))
                                  .setPStages(shader_stage_info.data())
                                  .setPVertexInputState(&vertex_input_state_create_info)
                                  .setPInputAssemblyState(&input_assembly_state_create_info)
                                  .setPRasterizationState(&rasterization_state_create_info)
                                  .setPViewportState(&viewport_state_create_info)
                                  .setPMultisampleState(&multisample_state_create_info)
                                  .setPColorBlendState(&colour_blend_state_create_info)
                                  .setLayout(layout)
                                  .setRenderPass(m_info.render_pass)
                                  .setSubpass(0)
                                  .setBasePipelineHandle(nullptr);

      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createGraphicsPipeline(nullptr, create_info);
             })
         .left_map([](auto&& err) {
            return make_error(error::failed_to_create_pipeline, err.code());
         })
         .right_map([&](auto&& handle) {
            util::log_info(m_plogger, R"([vkn] graphics pipeline created)");

            return graphics_pipeline(
               {.device = m_info.device, .pipeline = handle, .pipeline_layout = layout});
         });
   }
} // namespace vkn
