#pragma once

#include <water_simulation/render/pipeline.hpp>
#include <water_simulation/render/render_system.hpp>

#include <vkn/descriptor_pool.hpp>

enum struct camera_error
{

};

auto to_string(camera_error err) -> std::string;
auto to_err_code(camera_error err) -> util::error_t;

class camera
{
public:
   struct matrices
   {
      glm::mat4 projection;
      glm::mat4 view;
   };

   struct create_info
   {
      render_system& renderer;
      graphics_pipeline& pipeline;

      util::count32_t image_count{};

      util::logger_wrapper logger{};
   };

   static auto make(create_info&& info) -> util::result<camera>;

public:
   void update(util::index_t image_index, const matrices& matrices);

   auto lookup_set(util::index_t image_index) -> vk::DescriptorSet;

private:
   vkn::descriptor_pool m_descriptor_pool;

   crl::dynamic_array<vkn::buffer> m_uniform_buffers;
};

auto create_camera(render_system& system, graphics_pipeline& pipeline, util::logger_wrapper logger)
   -> camera;
auto create_offscreen_camera(render_system& system, graphics_pipeline& pipeline,
                             util::logger_wrapper logger) -> camera;
