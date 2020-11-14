#include <vkn/pipeline.hpp>

#include <monads/try.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>

#include <cassert>
#include <span>
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
      auto check_vertex_attribute_support(
         const vkn::shader* p_shader,
         std::span<const vk::VertexInputAttributeDescription> attributes,
         const std::shared_ptr<util::logger>& p_logger) -> bool
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
               util::log_error(p_logger,
                               "[vkn] vertex shader attribute at location {} and binding {} is not "
                               "supported by the shader",
                               attrib.location, attrib.binding);

               return false;
            }
         }

         return true;
      }

   }; // namespace detail

   struct graphics_pipeline_data
   {
      vk::UniquePipeline pipeline{nullptr};
      vk::UniquePipelineLayout pipeline_layout{nullptr};

      std::unordered_map<std::string, vkn::descriptor_set_layout> set_layouts{};
      std::unordered_map<std::string, vk::PushConstantRange> push_constants{};
   };

   auto create_pipeline(const graphics_pipeline::create_info& create_info,
                        graphics_pipeline_data&& data,
                        const std::shared_ptr<util::logger>& p_logger)
      -> util::result<graphics_pipeline_data>
   {
      util::dynamic_array<vk::PipelineShaderStageCreateInfo> shader_stage_info{};
      shader_stage_info.reserve(std::size(create_info.shader_infos));

      util::index_t vertex_shader_index{std::numeric_limits<std::size_t>::max()};
      for (std::uint32_t index = 0; const auto& info : create_info.shader_infos)
      {
         util::log_info(p_logger, R"([vkn] using shader "{0}" for graphics pipeline)",
                        info.p_shader->name());

         shader_stage_info.emplace_back(
            vk::PipelineShaderStageCreateInfo{}
               .setPNext(nullptr)
               .setFlags({})
               .setPSpecializationInfo(nullptr)
               .setStage(detail::to_shader_stage_flag(info.p_shader->stage()))
               .setModule(info.p_shader->value())
               .setPName("main"));

         if (info.p_shader->stage() == shader_type::vertex)
         {
            vertex_shader_index = index;
         }

         ++index;
      }

      const auto* p_vertex_shader = create_info.shader_infos[vertex_shader_index.value()].p_shader;
      if (!detail::check_vertex_attribute_support(p_vertex_shader, create_info.attributes,
                                                  create_info.p_logger))
      {
         return monad::err(to_err_code(graphics_pipeline_error::invalid_vertex_shader_bindings));
      }

      const auto vertex_input_state_create_info =
         vk::PipelineVertexInputStateCreateInfo{}
            .setVertexBindingDescriptionCount(std::size(create_info.bindings))
            .setPVertexBindingDescriptions(std::data(create_info.bindings))
            .setVertexAttributeDescriptionCount(std::size(create_info.attributes))
            .setPVertexAttributeDescriptions(std::data(create_info.attributes));

      const auto input_assembly_state_create_info =
         vk::PipelineInputAssemblyStateCreateInfo{}
            .setTopology(vk::PrimitiveTopology::eTriangleList)
            .setPrimitiveRestartEnable(false);

      const auto viewport_state_create_info = vk::PipelineViewportStateCreateInfo{}
                                                 .setViewportCount(std::size(create_info.viewports))
                                                 .setPViewports(create_info.viewports.data())
                                                 .setScissorCount(std::size(create_info.scissors))
                                                 .setPScissors(create_info.scissors.data());

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

      const auto info = vk::GraphicsPipelineCreateInfo{}
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
                           .setLayout(data.pipeline_layout.get())
                           .setRenderPass(create_info.render_pass.value())
                           .setSubpass(0)
                           .setBasePipelineHandle(nullptr);

      auto logical_device = create_info.device.logical_device();
      return monad::try_wrap<vk::SystemError>([&] {
                return logical_device.createGraphicsPipelineUnique(nullptr, info);
             })
         .map_error([]([[maybe_unused]] auto&& err) {
            return to_err_code(graphics_pipeline_error::failed_to_create_pipeline);
         })
         .map([&](vk::UniquePipeline&& handle) {
            util::log_info(p_logger, R"([vkn] graphics pipeline created)");

            data.pipeline = std::move(handle);

            return std::move(data);
         });
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
               vk::PushConstantRange{.stageFlags = to_shader_flag(info.p_shader->stage()),
                                     .offset = static_cast<uint32_t>(push.offset.value()),
                                     .size = static_cast<uint32_t>(push.size.value())});
         }
      }

      return ranges;
   }

   auto create_pipeline_layout(const vkn::device& device, graphics_pipeline_data&& pipeline,
                               const std::shared_ptr<util::logger>& p_logger)
      -> util::result<graphics_pipeline_data>
   {
      util::dynamic_array<vk::DescriptorSetLayout> layouts;
      layouts.reserve(std::size(pipeline.set_layouts));

      for (const auto& [name, layout] : pipeline.set_layouts)
      {
         layouts.emplace_back(layout.value());
      }

      util::dynamic_array<vk::PushConstantRange> push_constants;
      push_constants.reserve(std::size(pipeline.push_constants));

      for (const auto& [name, push] : pipeline.push_constants)
      {
         push_constants.emplace_back(push);
      }

      return monad::try_wrap<vk::SystemError>([&] {
                return device.logical_device().createPipelineLayoutUnique(
                   {.pNext = nullptr,
                    .flags = {},
                    .setLayoutCount = static_cast<std::uint32_t>(std::size(layouts)),
                    .pSetLayouts = std::data(layouts),
                    .pushConstantRangeCount = static_cast<std::uint32_t>(std::size(push_constants)),
                    .pPushConstantRanges = std::data(push_constants)});
             })
         .map([&](vk::UniquePipelineLayout&& layout) {
            pipeline.pipeline_layout = std::move(layout);

            util::log_debug(p_logger, "[vkn] graphics pipeline layout created");

            return std::move(pipeline);
         })
         .map_error([]([[maybe_unused]] const auto& err) {
            return to_err_code(graphics_pipeline_error::failed_to_create_pipeline_layout);
         });
   }

   auto create_descriptor_set_layouts(const vkn::device& device,
                                      std::span<const pipeline_shader_data> shader_infos,
                                      const std::shared_ptr<util::logger>& p_logger)
      -> util::result<graphics_pipeline_data>
   {
      graphics_pipeline_data data;
      data.push_constants = populate_push_constants(shader_infos);

      for (const auto& shader_info : shader_infos)
      {
         for (const auto& set_info : shader_info.set_layouts)
         {
            auto bindings =
               set_info.bindings | ranges::views::transform([&](set_layout_binding binding) {
                  vk::DescriptorSetLayoutBinding result;
                  result.binding = binding.binding.value();
                  result.descriptorType = binding.descriptor_type;
                  result.descriptorCount = binding.descriptor_count.value();
                  result.stageFlags = detail::to_shader_stage_flag(shader_info.p_shader->stage());

                  return result;
               });

            auto result = vkn::descriptor_set_layout::builder{device, p_logger}
                             .set_bindings(bindings | ranges::to<util::dynamic_array>)
                             .build();

            if (result)
            {
               data.set_layouts.insert_or_assign(set_info.name, std::move(result).value().value());
            }
            else
            {
               return monad::err(result.error().value());
            }
         }
      }

      return data;
   }

   auto graphics_pipeline::make(create_info&& info) -> util::result<graphics_pipeline>
   {
      const auto finalize = [](graphics_pipeline_data&& data) {
         graphics_pipeline pipeline;
         pipeline.m_pipeline = std::move(data.pipeline);
         pipeline.m_pipeline_layout = std::move(data.pipeline_layout);
         pipeline.m_set_layouts = std::move(data.set_layouts);
         pipeline.m_push_constants = std::move(data.push_constants);

         return pipeline;
      };

      return create_descriptor_set_layouts(info.device, info.shader_infos, info.p_logger)
         .and_then([&](graphics_pipeline_data&& data) {
            return create_pipeline_layout(info.device, std::move(data), info.p_logger);
         })
         .and_then([&](graphics_pipeline_data&& data) {
            return create_pipeline(info, std::move(data), info.p_logger);
         })
         .map(finalize);
   }

   auto graphics_pipeline::value() const noexcept -> vk::Pipeline { return m_pipeline.get(); }
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
} // namespace vkn
