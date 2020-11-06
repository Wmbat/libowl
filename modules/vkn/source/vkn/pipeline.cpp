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
         case err_t::invalid_vertex_shader_bindings:
            return "invalid_vertex_shader_bindings";
         case err_t::failed_to_create_pipeline_layout:
            return "failed_to_create_pipeline_layout";
         default:
            return "UNKNOWN";
      }
   }
   auto make_error(graphics_pipeline_error err) -> vkn::error_t
   {
      return {{static_cast<int>(err), graphics_pipeline_category}};
   }

   auto graphics_pipeline::layout() const noexcept -> vk::PipelineLayout
   {
      return m_pipeline_layout.get();
   }
   auto graphics_pipeline::device() const noexcept -> vk::Device { return m_value.getOwner(); }
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

   graphics_pipeline::builder::builder(const vkn::device& device,
                                       const vkn::render_pass& render_pass,
                                       std::shared_ptr<util::logger> p_logger) :
      mp_logger{std::move(p_logger)}
   {
      m_info.device = device.logical_device();
      m_info.render_pass = render_pass.value();
   }

   auto graphics_pipeline::builder::build() const -> vkn::result<graphics_pipeline>
   {
      return create_descriptor_set_layouts()
         .and_then([&](auto data) {
            util::log_info(mp_logger, "[vkn] {} descriptor set layouts created",
                           std::size(m_info.set_layouts));

            return create_push_constant_ranges(std::move(data));
         })
         .and_then([&](auto data) {
            util::log_info(mp_logger, "[vkn] {} push constant ranges created",
                           std::size(m_info.push_constants));

            return create_pipeline_layout(std::move(data));
         })
         .and_then([&](auto info) {
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
      const std::string& name, const util::dynamic_array<vk::DescriptorSetLayoutBinding>& binding)
      -> builder&
   {
      m_info.set_layouts.emplace_back(descriptor_set_layout_info{.name = name, .binding = binding});
      return *this;
   }

   auto graphics_pipeline::builder::add_push_constant(const std::string& name,
                                                      vkn::shader_type shader_type,
                                                      util::size_t offset, util::size_t size)
      -> builder&
   {
      m_info.push_constants.emplace_back(
         push_constant_info{.name = name, .type = shader_type, .offset = offset, .size = size});
      return *this;
   }

   auto graphics_pipeline::builder::create_descriptor_set_layouts() const
      -> vkn::result<graphics_pipeline>
   {
      graphics_pipeline pipeline;
      pipeline.m_set_layouts.reserve(std::size(m_info.set_layouts));

      for (const auto& set_info : m_info.set_layouts)
      {
         auto result = vkn::descriptor_set_layout::builder{m_info.device, mp_logger}
                          .set_bindings(set_info.binding)
                          .build();

         if (result)
         {
            pipeline.m_set_layouts.insert_or_assign(set_info.name,
                                                    std::move(result).value().value());
         }
         else
         {
            return monad::err(result.error().value());
         }
      }

      return pipeline;
   }

   auto graphics_pipeline::builder::create_push_constant_ranges(graphics_pipeline&& pipeline) const
      -> vkn::result<graphics_pipeline>
   {
      pipeline.m_push_constants.reserve(std::size(m_info.push_constants));

      for (const auto& push : m_info.push_constants)
      {
         pipeline.m_push_constants.insert_or_assign(
            push.name,
            vk::PushConstantRange{.stageFlags = to_shader_flag(push.type),
                                  .offset = static_cast<uint32_t>(push.offset.value()),
                                  .size = static_cast<uint32_t>(push.size.value())});
      }

      return std::move(pipeline);
   }

   auto graphics_pipeline::builder::create_pipeline_layout(graphics_pipeline&& pipeline) const
      -> vkn::result<graphics_pipeline>
   {
      util::dynamic_array<vk::DescriptorSetLayout> layouts;
      layouts.reserve(std::size(pipeline.m_set_layouts));

      for (const auto& [name, layout] : pipeline.m_set_layouts)
      {
         layouts.emplace_back(layout.value());
      }

      util::dynamic_array<vk::PushConstantRange> push_constants;
      push_constants.reserve(std::size(pipeline.m_push_constants));

      for (const auto& [name, push] : pipeline.m_push_constants)
      {
         push_constants.emplace_back(push);
      }

      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createPipelineLayoutUnique(
                   {.pNext = nullptr,
                    .flags = {},
                    .setLayoutCount = static_cast<std::uint32_t>(std::size(layouts)),
                    .pSetLayouts = std::data(layouts),
                    .pushConstantRangeCount = static_cast<std::uint32_t>(std::size(push_constants)),
                    .pPushConstantRanges = std::data(push_constants)});
             })
         .map([&](vk::UniquePipelineLayout&& layout) {
            pipeline.m_pipeline_layout = std::move(layout);

            return std::move(pipeline);
         })
         .map_error([]([[maybe_unused]] const auto& err) {
            return make_error(graphics_pipeline_error::failed_to_create_pipeline_layout);
         });
   }

   auto graphics_pipeline::builder::create_pipeline(graphics_pipeline&& pipeline) const
      -> vkn::result<graphics_pipeline>
   {
      shader_dynamic_array<vk::PipelineShaderStageCreateInfo> shader_stage_info{};
      shader_stage_info.reserve(std::size(m_info.shaders));

      util::index_t vertex_shader_index{std::numeric_limits<std::size_t>::max()};
      for (std::uint32_t index = 0; const auto* shader_handle : m_info.shaders)
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

         if (shader_handle->stage() == shader_type::vertex)
         {
            vertex_shader_index = index;
         }

         ++index;
      }

      const auto* p_vertex_shader = m_info.shaders[vertex_shader_index.value()];
      if (!check_vertex_attribute_support(p_vertex_shader))
      {
         return monad::err(make_error(graphics_pipeline_error::invalid_vertex_shader_bindings));
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
                                  .setLayout(pipeline.m_pipeline_layout.get())
                                  .setRenderPass(m_info.render_pass)
                                  .setSubpass(0)
                                  .setBasePipelineHandle(nullptr);

      return monad::try_wrap<vk::SystemError>([&] {
                return m_info.device.createGraphicsPipelineUnique(nullptr, create_info);
             })
         .map_error([]([[maybe_unused]] auto&& err) {
            return make_error(graphics_pipeline_error::failed_to_create_pipeline);
         })
         .map([&](vk::UniquePipeline&& handle) {
            util::log_info(mp_logger, R"([vkn] graphics pipeline created)");

            pipeline.m_value = std::move(handle);

            return std::move(pipeline);
         });
   }

   auto graphics_pipeline::builder::check_vertex_attribute_support(
      const vkn::shader* p_shader) const noexcept -> bool
   {
      const auto& data = p_shader->get_data();
      for (const auto& attrib : m_info.attribute_descriptions)
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
            util::log_error(mp_logger,
                            "[vkn] vertex shader attribute at location {} and binding {} is not "
                            "supported by the shader",
                            attrib.location, attrib.binding);

            return false;
         }
      }

      return true;
   }
} // namespace vkn
