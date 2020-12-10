#pragma once

#include <water_simulation/collision/primitive.hpp>
#include <water_simulation/collision/system.hpp>
#include <water_simulation/core.hpp>
#include <water_simulation/particle.hpp>
#include <water_simulation/render/camera.hpp>
#include <water_simulation/render/offscreen.hpp>
#include <water_simulation/render/pipeline.hpp>
#include <water_simulation/render/pipeline_registry.hpp>
#include <water_simulation/render/render_system.hpp>
#include <water_simulation/render/renderable.hpp>
#include <water_simulation/render/shader_registry.hpp>
#include <water_simulation/sph/system.hpp>

#include <ui/window.hpp>

#include <entt/entt.hpp>

#include <future>

class simulation
{
public:
   simulation(const settings& settings);

   void run();

private:
   void update();
   void render();

   void onscreen_render();
   void offscreen_render();

   void setup_offscreen();

   auto create_main_pipeline() -> pipeline_index_t;
   auto create_offscreen_pipeline() -> pipeline_index_t;

   auto setup_onscreen_camera(pipeline_index_t index) -> camera;
   auto setup_offscreen_camera(pipeline_index_t index) -> camera;

   auto compute_matrices(std::uint32_t width, std::uint32_t height) -> camera::matrices;

   void add_invisible_wall(const glm::vec3& position, const glm::vec3& dimensions);
   void add_box(const glm::vec3& position, const glm::vec3& dimensions, const glm::vec3& colour);

   void write_image_to_disk(std::string_view name);

   template <typename Any>
   constexpr auto check_err(Any&& result)
   {
      return handle_err(std::forward<Any>(result), &m_logger);
   }

private:
   util::logger m_logger;

   settings m_settings;

   ui::window m_window;

   render_system m_render_system;

   camera m_camera;

   shader_registry m_shaders;
   pipeline_registry m_pipelines;

   util::dynamic_array<render_pass> m_render_passes;

   pipeline_index_t m_main_pipeline_key{};
   pipeline_index_t m_offscreen_pipeline_key{};

   entt::registry m_registry;

   renderable m_sphere;
   renderable m_box;

   sph::system m_sph_system;
   collision::system m_collision_system{};

   util::dynamic_array<particle> m_particles;

   std::string m_vert_shader_key{"resources/shaders/test_vert.spv"};
   std::string m_frag_shader_key{"resources/shaders/test_frag.spv"};

   offscreen m_offscreen;

   util::dynamic_array<std::uint8_t> m_image_pixels;
   std::future<void> m_image_write_fut;

   float m_max_density{0};

   duration<float, std::milli> m_time_per_frame = 16ms;
   duration<float, std::milli> m_time_spent = 0ms;

   util::count64_t m_frame_count = 0;
};
