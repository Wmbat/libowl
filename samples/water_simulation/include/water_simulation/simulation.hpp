#pragma once

#include <water_simulation/collision/contact.hpp>
#include <water_simulation/collision/primitive.hpp>
#include <water_simulation/core.hpp>
#include <water_simulation/particle.hpp>
#include <water_simulation/render/camera.hpp>
#include <water_simulation/render/pipeline.hpp>
#include <water_simulation/render/pipeline_codex.hpp>
#include <water_simulation/render/render_system.hpp>
#include <water_simulation/render/renderable.hpp>
#include <water_simulation/render/shader_codex.hpp>

#include <ui/window.hpp>

class simulation
{
public:
   simulation(const settings& settings);

   void run();

private:
   void update();
   void render();

   auto create_main_pipeline() -> pipeline_index_t;
   auto setup_camera(pipeline_index_t index) -> camera;
   auto compute_matrices(const render_system& system) -> camera::matrices;

   void integrate(std::span<particle> particles);
   void resolve_collisions(std::span<particle> particles);

   auto compare_distance(float d0, float d1) -> bool;

   template <typename Any>
   constexpr auto check_err(Any&& result)
   {
      return handle_err(std::forward<Any>(result), m_logger);
   }

private:
   util::logger_ptr m_logger;

   settings m_settings;

   ui::window m_window;

   render_system m_render_system;

   camera m_camera;

   shader_codex m_shaders;
   pipeline_codex m_pipelines;

   util::dynamic_array<render_pass> m_render_passes;

   pipeline_index_t m_main_pipeline_key;

   renderable m_sphere;
   renderable m_box;

   util::dynamic_array<particle> m_particles;

   util::dynamic_array<collision::box> m_boxes;

   std::string m_vert_shader_key{"resources/shaders/test_vert.spv"};
   std::string m_frag_shader_key{"resources/shaders/test_frag.spv"};
};
