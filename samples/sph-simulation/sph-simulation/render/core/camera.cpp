#include <sph-simulation/render/core/camera.hpp>

#include <sph-simulation/core.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

namespace vi = ranges::views;

auto create_uniform_buffers(const camera_create_info& info) -> std::vector<cacao::buffer>
{
   const auto create_info =
      cacao::buffer_create_info{.device = info.device,
                                .buffer_size = sizeof(camera::matrices),
                                .usage = vk::BufferUsageFlagBits::eUniformBuffer,
                                .desired_mem_flags = vk::MemoryPropertyFlagBits::eHostVisible |
                                   vk::MemoryPropertyFlagBits::eHostCoherent,
                                .logger = info.logger};

   // clang-format off

   return vi::iota(0u, info.image_count) 
      | vi::transform([&](auto) { return cacao::buffer(create_info); }) 
      | ranges::to_vector;

   // clang-format on
}

camera::camera(const camera_create_info& info) :
   m_uniform_buffers(create_uniform_buffers(info)),
   m_descriptor_pool({.device = info.device,
                      .pool_sizes = {{.type = vk::DescriptorType::eUniformBuffer,
                                      .descriptorCount = info.image_count}},
                      .layouts = std::vector(info.image_count, info.layout.value()),
                      .logger = info.logger}),
   m_device(info.device.logical())
{
   for (std::size_t i = 0; auto set : m_descriptor_pool.sets())
   {
      const std::array buf_info = {
         vk::DescriptorBufferInfo{.buffer = m_uniform_buffers.at(i++).value(),
                                  .offset = 0,
                                  .range = sizeof(camera_matrices)}};

      vk::WriteDescriptorSet write{.dstSet = set,
                                   .dstBinding = 0,
                                   .dstArrayElement = 0,
                                   .descriptorCount = std::size(buf_info),
                                   .descriptorType = vk::DescriptorType::eUniformBuffer,
                                   .pBufferInfo = std::data(buf_info)};

      info.device.logical().updateDescriptorSets({write}, {});
   }
}

auto camera::lookup_set(mannele::u64 image_index) -> vk::DescriptorSet
{
   return m_descriptor_pool.sets()[image_index];
}

void camera::update(mannele::u64 image_index, const matrices& matrices)
{
   constexpr auto size = sizeof(matrices);

   auto memory = m_uniform_buffers.at(image_index).memory();

   void* p_data = m_device.mapMemory(memory, 0, size, {});
   memcpy(p_data, &matrices, size);
   m_device.unmapMemory(m_uniform_buffers.at(image_index).memory());
}
