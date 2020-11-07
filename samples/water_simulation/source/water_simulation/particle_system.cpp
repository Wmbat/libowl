#include <water_simulation/particle_system.hpp>

#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include <glm/ext/matrix_transform.hpp>

#include <utility>

util::dynamic_array<gfx::vertex> m_particle_vertices{{{-0.5F, -0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}},
                                                     {{0.5F, -0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}},
                                                     {{0.5F, 0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}},
                                                     {{-0.5F, 0.5F, 0.0F}, {1.0F, 0.0F, 0.0F}}};

util::dynamic_array<std::uint32_t> m_particle_indices{0, 1, 2, 2, 3, 0};

auto is_active(ranges::common_pair<std::size_t, particle&> pair) -> bool
{
   return pair.second.is_active;
}

namespace vi = ranges::views;

particle_engine::particle_engine(gfx::render_manager& render_manager, std::size_t particle_count) :
   m_render_manager{render_manager}, m_particle_index{particle_count - 1}
{
   m_particles.resize(particle_count);

   for (std::size_t i : vi::iota(0U, particle_count))
   {
      std::string name = "particle_" + std::to_string(i);

      m_render_manager.subscribe_renderable(
         name,
         {.vertices = m_particle_vertices,
          .indices = m_particle_indices,
          .model = glm::translate(glm::mat4{1}, {1000.0F, 0.0f, 0.0f})});

      m_names.push_back(name);
   }
}

void particle_engine::update(const std::function<void(particle&)>& fun)
{
   for (auto [index, particle] : m_particles | vi::enumerate | vi::filter(is_active))
   {
      std::invoke(fun, particle);

      const auto model =
         glm::translate(glm::scale(glm::mat4{1.0F}, {1 / 5.0F, 1 / 5.0F, 0.0F}), particle.position);
      m_render_manager.update_model_matrix(m_names[index], model);
   }
}

void particle_engine::emit(const particle& particle)
{
   // m_particles[m_particle_index] = particle;
   // m_particles[m_particle_index].is_active = true;
}
