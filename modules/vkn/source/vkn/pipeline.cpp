#include <vkn/pipeline.hpp>

#include <monads/try.hpp>

#include <cassert>
#include <ranges>
#include <utility>

namespace vkn
{
   namespace detail
   {
      auto to_shader_stage_flag(vkn::shader_type stage) noexcept -> vk::ShaderStageFlagBits
      {
         switch (stage)
         {
            case shader_type::vertex:
               return vk::ShaderStageFlagBits::eVertex;
            case shader_type::fragment:
               return vk::ShaderStageFlagBits::eFragment;
            case shader_type::compute:
               return vk::ShaderStageFlagBits::eCompute;
            case shader_type::geometry:
               return vk::ShaderStageFlagBits::eGeometry;
            case shader_type::tess_eval:
               return vk::ShaderStageFlagBits::eTessellationEvaluation;
            case shader_type::tess_control:
               return vk::ShaderStageFlagBits::eTessellationControl;
            default:
               return {};
         }
      }
   }; // namespace detail

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
      }
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
         case err_t::failed_to_create_pipeline_layout:
            return "failed_to_create_pipeline_layout";
         default:
            return "UNKNOWN";
      }
   }
   auto make_error(graphics_pipeline_error err, std::error_code ec) -> vkn::error
   {
      return {{static_cast<int>(err), graphics_pipeline_category},
              static_cast<vk::Result>(ec.value())};
   }

   auto graphics_pipeline::layout() const noexcept -> vk::PipelineLayout
   {
      return m_pipeline_layout.get();
   }
   auto graphics_pipeline::device() const noexcept -> vk::Device { return m_value.getOwner(); }
   auto graphics_pipeline::descriptor_set_layouts() const noexcept
      -> util::dynamic_array<vk::DescriptorSetLayout>
   {
      util::dynamic_array<vk::DescriptorSetLayout> set_layouts{};
      set_layouts.reserve(std::size(m_set_layouts));

      for (const auto& layout : m_set_layouts)
      {
         set_layouts.emplace_back(layout.get());
      }

      return set_layouts;
   }

   graphics_pipeline::builder::builder(const vkn::device& device,
                                       const vkn::render_pass& render_pass,
                                       std::shared_ptr<util::logger> p_logger) :
      mp_logger{std::move(p_logger)}
   {
      m_info.device = device.value();
      m_info.render_pass = render_pass.value();
   }

   auto graphics_pipeline::builder::build() const -> vkn::result<graphics_pipeline>
   {
      return create_descriptor_set_layouts()
         .and_then([&](util::dynamic_array<vk::UniqueDescriptorSetLayout>&& data) {
            util::log_info(mp_logger, "[vkn] {} descriptor set layouts created",
                           std::size(m_info.set_layouts));

            return create_pipeline_layout(std::move(data));
         })
         .and_then([&](layout_info&& info) {
            util::log_info(mp_logger, "[vkn] pipeline layout created");

            return create_pipeline(std::move(info));
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
   auto graphics_pipeline::builder::add_vertex_binding(
      vk::VertexInputBindingDescription&& binding) noexcept -> builder&
   {
      m_info.binding_descriptions.emplace_back(binding);
      return *this;
   }
   auto graphics_pipeline::builder::add_vertex_attribute(
      vk::VertexInputAttributeDescription&& attribute) noexcept -> builder&
   {
      m_info.attribute_descriptions.emplace_back(attribute);
      return *this;
   }
   auto graphics_pipeline::builder::add_set_layout(
      const util::dynamic_array<vk::DescriptorSetLayoutBinding>& binding) -> builder&
   {
      m_info.set_layouts.emplace_back(descriptor_set_layout_info{.binding = binding});
      return *this;
   }

   auto graphics_pipeline::builder::create_descriptor_set_layouts() const
      -> vkn::result<util::dynamic_array<vk::UniqueDescriptorSetLayout>>
   {
      util::dynamic_array<vk::UniqueDescriptorSetLayout> layouts;
      layouts.reserve(std::size(m_info.set_layouts));

      for (const auto& set_info : m_info.set_layouts)
      {
         auto result = try_wrap([&] {
                          return m_info.device.createDescriptorSetLayoutUnique(
                             {.bindingCount = static_cast<uint32_t>(std::size(set_info.binding)),
                              .pBindings = set_info.binding.data()});
                       }).map_error([](const vk::SystemError& err) {
            return make_error(graphics_pipeline_error::failed_to_create_descriptor_set_layout,
                              err.code());
         });

         if (result)
         {
            layouts.emplace_back(std::move(result).value().value());
         }
         else
         {
            return monad::make_error(result.error().value());
         }
      }

      return layouts;
   }
   auto graphics_pipeline::builder::create_pipeline_layout(
      util::dynamic_array<vk::UniqueDescriptorSetLayout>&& set_layouts) const
      -> vkn::result<layout_info>
   {
      util::dynamic_array<vk::DescriptorSetLayout> layouts;
      layouts.reserve(std::size(set_layouts));

      for (const auto& layout : set_layouts)
      {
         layouts.emplace_back(layout.get());
      }

      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createPipelineLayoutUnique(
                   {.pNext = nullptr,
                    .flags = {},
                    .setLayoutCount = static_cast<uint32_t>(std::size(layouts)),
                    .pSetLayouts = std::data(layouts),
                    .pushConstantRangeCount = 0u,
                    .pPushConstantRanges = nullptr});
             })
         .map([&](vk::UniquePipelineLayout&& layout) {
            return layout_info{.set_layouts = std::move(set_layouts),
                               .pipeline_layout = std::move(layout)};
         })
         .map_error([](vk::SystemError&& err) {
            return make_error(graphics_pipeline_error::failed_to_create_pipeline_layout,
                              err.code());
         });
   }

   auto graphics_pipeline::builder::create_pipeline(layout_info&& layout) const
      -> vkn::result<graphics_pipeline>
   {
      shader_dynamic_array<vk::PipelineShaderStageCreateInfo> shader_stage_info{};
      shader_stage_info.reserve(std::size(m_info.shaders));

      for (const auto* shader_handle : m_info.shaders)
      {
         util::log_info(mp_logger, R"([vkn] using shader "{0}" for graphics pipeline)",
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

      const auto vertex_input_state_create_info =
         vk::PipelineVertexInputStateCreateInfo{}
            .setVertexBindingDescriptionCount(std::size(m_info.binding_descriptions))
            .setPVertexBindingDescriptions(std::data(m_info.binding_descriptions))
            .setVertexAttributeDescriptionCount(std::size(m_info.attribute_descriptions))
            .setPVertexAttributeDescriptions(std::data(m_info.attribute_descriptions));

      const auto input_assembly_state_create_info =
         vk::PipelineInputAssemblyStateCreateInfo{}
            .setTopology(vk::PrimitiveTopology::eTriangleList)
            .setPrimitiveRestartEnable(false);

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
                                  .setLayout(layout.pipeline_layout.get())
                                  .setRenderPass(m_info.render_pass)
                                  .setSubpass(0)
                                  .setBasePipelineHandle(nullptr);

      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createGraphicsPipelineUnique(nullptr, create_info);
             })
         .map_error([](auto&& err) {
            return make_error(graphics_pipeline_error::failed_to_create_pipeline, err.code());
         })
         .map([&](vk::UniquePipeline&& handle) {
            util::log_info(mp_logger, R"([vkn] graphics pipeline created)");

            graphics_pipeline pipeline{};
            pipeline.m_value = std::move(handle);
            pipeline.m_pipeline_layout = std::move(layout.pipeline_layout);
            pipeline.m_set_layouts = std::move(layout.set_layouts);

            return pipeline;
         });
   }
} // namespace vkn
