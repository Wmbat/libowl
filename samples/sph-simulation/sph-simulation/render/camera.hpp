#pragma once

#include <sph-simulation/render/pipeline.hpp>
#include <sph-simulation/render/render_system.hpp>

#include <libcacao/descriptor_pool.hpp>

struct camera_create_info
{
   render_system& renderer;
   graphics_pipeline& pipeline;

   mannele::u32 image_count{};

   util::log_ptr logger{};
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

   void update(cacao::index_t image_index, const matrices& matrices);

   auto lookup_set(cacao::index_t image_index) -> vk::DescriptorSet;

private:
   std::vector<cacao::buffer> m_uniform_buffers;

   cacao::descriptor_pool m_descriptor_pool;

   vk::Device m_device;
};

auto create_camera(render_system& system, graphics_pipeline& pipeline, util::log_ptr logger)
   -> camera;
auto create_offscreen_camera(render_system& system, graphics_pipeline& pipeline,
                             util::log_ptr logger) -> camera;
