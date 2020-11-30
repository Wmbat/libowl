#pragma once

#include <water_simulation/collision/primitive.hpp>
#include <water_simulation/collision/system.hpp>
#include <water_simulation/core.hpp>
#include <water_simulation/particle.hpp>
#include <water_simulation/render/camera.hpp>
#include <water_simulation/render/pipeline.hpp>
#include <water_simulation/render/pipeline_registry.hpp>
#include <water_simulation/render/render_system.hpp>
#include <water_simulation/render/renderable.hpp>
#include <water_simulation/render/shader_registry.hpp>
#include <water_simulation/sph/system.hpp>

#include <ui/window.hpp>

#include <entt/entt.hpp>

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

   void add_invisible_wall(const glm::vec3& position, const glm::vec3& dimensions);
   void add_box(const glm::vec3& position, const glm::vec3& dimensions, const glm::vec3& colour);

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

   pipeline_index_t m_main_pipeline_key;

   entt::registry m_registry;

   renderable m_sphere;
   renderable m_box;

   sph::system m_sph_system;
   collision::system m_collision_system{};

   util::dynamic_array<particle> m_particles;

   std::string m_vert_shader_key{"resources/shaders/test_vert.spv"};
   std::string m_frag_shader_key{"resources/shaders/test_frag.spv"};
};
