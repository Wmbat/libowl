#include <fluid_simulation/render/camera.hpp>

#include <fluid_simulation/core.hpp>

#include <range/v3/view/iota.hpp>

namespace vi = ranges::views;

struct camera_data
{
   const camera::create_info& info;

   vkn::descriptor_pool pool{};
   crl::dynamic_array<vkn::buffer> buffers{};
};

auto create_uniform_buffers(const camera::create_info& info) -> util::result<camera_data>
{
   crl::dynamic_array<vkn::buffer> buffers;
   buffers.reserve(info.image_count.value());

   for ([[maybe_unused]] std::uint32_t i : vi::iota(0U, info.image_count.value()))
   {
      auto res = vkn::buffer::builder{info.renderer.device(), info.logger}
                    .set_size(sizeof(camera::matrices))
                    .set_usage(vk::BufferUsageFlagBits::eUniformBuffer)
                    .set_desired_memory_type(vk::MemoryPropertyFlagBits::eHostVisible |
                                             vk::MemoryPropertyFlagBits::eHostCoherent)
                    .build();

      if (auto err = res.error())
      {
         return monad::err(err.value());
      }

      buffers.append(std::move(res).value().value());
   }

   return camera_data{.info = info, .buffers = std::move(buffers)};
}

auto create_descriptor_pool(camera_data&& data) -> util::result<camera_data>
{
   auto& info = data.info;

   return vkn::descriptor_pool::builder{info.renderer.device(), info.logger}
      .add_pool_size(vk::DescriptorType::eUniformBuffer, info.image_count)
      .set_descriptor_set_layout(info.pipeline.get_descriptor_set_layout("camera_layout").value())
      .set_max_sets(info.image_count)
      .build()
      .map([&](vkn::descriptor_pool&& pool) {
         data.pool = std::move(pool);

         for (std::size_t i = 0; auto set : data.pool.sets())
         {
            std::array buf_info{vk::DescriptorBufferInfo{.buffer = *data.buffers.lookup(i++),
                                                         .offset = 0,
                                                         .range = sizeof(cacao::camera_matrices)}};
            vk::WriteDescriptorSet write{.dstSet = set,
                                         .dstBinding = 0,
                                         .dstArrayElement = 0,
                                         .descriptorCount = std::size(buf_info),
                                         .descriptorType = vk::DescriptorType::eUniformBuffer,
                                         .pBufferInfo = std::data(buf_info)};

            info.renderer.device().logical().updateDescriptorSets({write}, {});
         }

         return std::move(data);
      });
}

auto camera::make(create_info&& info) -> util::result<camera>
{
   const auto finalize = [](camera_data&& data) {
      camera cam;
      cam.m_descriptor_pool = std::move(data.pool);
      cam.m_uniform_buffers = std::move(data.buffers);

      return cam;
   };

   return create_uniform_buffers(info).and_then(create_descriptor_pool).map(finalize);
}

auto camera::lookup_set(util::index_t image_index) -> vk::DescriptorSet
{
   return m_descriptor_pool.sets().lookup(image_index.value());
}

void camera::update(util::index_t image_index, const matrices& matrices)
{
   constexpr auto size = sizeof(matrices);

   auto device = m_descriptor_pool.device();

   void* p_data =
      device.mapMemory(m_uniform_buffers.lookup(image_index.value()).memory(), 0, size, {});
   memcpy(p_data, &matrices, size);
   device.unmapMemory(m_uniform_buffers.lookup(image_index.value()).memory());
}

auto create_camera(render_system& system, graphics_pipeline& pipeline, util::logger_wrapper logger)
   -> camera
{
   auto& config = system.lookup_configuration();

   return handle_err(camera::make({.renderer = system,
                                   .pipeline = pipeline,
                                   .image_count = config.swapchain_image_count,
                                   .logger = logger}),
                     logger);
}
auto create_offscreen_camera(render_system& system, graphics_pipeline& pipeline,
                             util::logger_wrapper logger) -> camera
{
   return handle_err(
      camera::make({.renderer = system, .pipeline = pipeline, .image_count = 1, .logger = logger}),
      logger);
}
