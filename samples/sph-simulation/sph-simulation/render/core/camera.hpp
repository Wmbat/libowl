#pragma once

#include <sph-simulation/core/pipeline.hpp>

#include <libcacao/buffer.hpp>
#include <libcacao/descriptor_pool.hpp>

#include <glm/mat4x4.hpp>

struct camera_create_info
{
   const cacao::device& device;
   const cacao::descriptor_set_layout& layout;

   mannele::u32 image_count{};

   mannele::log_ptr logger{};
};

struct camera_matrices
{
   glm::mat4 perspective;
   glm::mat4 view;
};

class camera
{
public:
   struct matrices
   {
      glm::mat4 projection;
      glm::mat4 view;
   };

public:
   camera() = default;
   camera(const camera_create_info& info);

   void update(mannele::u64 image_index, const matrices& matrices);

   auto lookup_set(mannele::u64 image_index) -> vk::DescriptorSet;

private:
   std::vector<cacao::buffer> m_uniform_buffers;

   cacao::descriptor_pool m_descriptor_pool;

   vk::Device m_device;
};
