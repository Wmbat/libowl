#pragma once

#include <sph-simulation/core.hpp>
#include <sph-simulation/core/pipeline.hpp>
#include <sph-simulation/core/pipeline_registry.hpp>
#include <sph-simulation/core/shader_registry.hpp>
#include <sph-simulation/render/core/camera.hpp>
#include <sph-simulation/render/offscreen.hpp>
#include <sph-simulation/render/render_system.hpp>
#include <sph-simulation/render/renderable.hpp>
#include <sph-simulation/sim_config.hpp>

#include <libcacao/window.hpp>

#include <entt/entt.hpp>

#include <future>

class simulation
{
public:
   simulation(sim_config config, util::log_ptr logger);

   void run();

private:
   void update();
   void render();

   void onscreen_render();
   void offscreen_render();

   void setup_offscreen();

   auto create_main_pipeline() -> mannele::u64;
   auto create_offscreen_pipeline() -> mannele::u64;

   auto setup_onscreen_camera(mannele::u64 index) -> camera;
   auto setup_offscreen_camera(mannele::u64 index) -> camera;

   auto compute_matrices(std::uint32_t width, std::uint32_t height) -> camera::matrices;

   void add_plane(float offset, const glm::vec3& normal);

   void write_image_to_disk(std::string_view name);

   template <typename Any>
   constexpr auto check_err(Any&& result)
   {
      return handle_err(std::forward<Any>(result), &m_logger);
   }

private:
   util::log_ptr m_logger;

   sim_config m_config;

   cacao::window m_window;

   render_system m_render_system;

   camera m_camera;

   shader_registry m_shaders;
   pipeline_registry m_pipelines;

   std::vector<render_pass> m_render_passes;

   mannele::u64 m_main_pipeline_key{};
   mannele::u64 m_offscreen_pipeline_key{};

   entt::registry m_registry;

   renderable m_plane;
   renderable m_sphere;
   renderable m_box;

   std::string m_vert_shader_key{"shaders/test_vert.spv"};
   std::string m_frag_shader_key{"shaders/test_frag.spv"};

   offscreen m_offscreen;

   std::vector<mannele::u8> m_image_pixels;
   bool has_offscreen_render{false};
   std::future<void> m_image_write_fut;

   duration<float, std::milli> m_time_per_frame = 16ms;
   duration<float, std::milli> m_time_spent = 0ms;

   mannele::u64 m_frame_count = 0;
};
