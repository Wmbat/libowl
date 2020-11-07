#include <water_simulation/particle_system.hpp>
#include <water_simulation/shader_codex.hpp>

#include <gfx/data_types.hpp>
#include <gfx/render_manager.hpp>
#include <gfx/window.hpp>

#include <ui/window.hpp>

#include <glm/ext/matrix_transform.hpp>

static constexpr float time_step = 0.0001F;
static constexpr float gravity = -9.81F;

void integrate([[maybe_unused]] particle& p)
{
   p.velocity = p.velocity + time_step * p.force / p.mass;
   p.position = p.position + time_step * p.velocity;
};

auto main() -> int
{
   auto main_logger = std::make_shared<util::logger>("water_simulation");

   glfwInit();

   ui::window window{"Water Simulation", 1080, 720};
   gfx::render_manager rendering_manager{window, main_logger};

   shader_codex codex{rendering_manager, main_logger};
   auto vert_res = codex.insert("resources/shaders/test_vert.spv", vkn::shader_type::vertex);
   auto frag_res = codex.insert("resources/shaders/test_frag.spv", vkn::shader_type::fragment);

   if (auto err = vert_res.error())
   {
      util::log_error(main_logger, "error: {}", err->value().message());
   }

   if (auto err = frag_res.error())
   {
      util::log_error(main_logger, "error: {}", err->value().message());
   }

   const auto vert_shader_info = vert_res.value().value();
   const auto frag_shader_info = frag_res.value().value();

   rendering_manager.bake(vert_shader_info.value(), frag_shader_info.value());

   particle_engine particle_engine{rendering_manager, 100};

   particle_engine.emit(
      {.position = {0.0F, 0.0F, 0.0F}, .force = {0.0F, gravity, 0.0F}, .is_active = true});

   while (window.is_open())
   {
      window.poll_events();

      particle_engine.update(integrate);

      rendering_manager.render_frame();
   }

   rendering_manager.wait();

   return 0;
}
