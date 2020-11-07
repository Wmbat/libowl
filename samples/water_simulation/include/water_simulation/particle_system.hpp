#pragma once

#include <gfx/render_manager.hpp>

#include <util/containers/dynamic_array.hpp>

#include <glm/vec3.hpp>

static constexpr std::size_t default_particle_count = 1000U;

struct particle
{
   glm::vec3 position{};
   glm::vec3 velocity{};
   glm::vec3 force{};

   float scale{1.0F};
   float mass{1.0F};
   float lifetime{1.0F};
   float life_remaining{0.0F};

   bool is_active{false};
};

class particle_engine
{
public:
   particle_engine(gfx::render_manager& render_manager,
                   std::size_t particle_count = default_particle_count);

   void update(const std::function<void(particle&)>& fun);

   void emit(const particle& particle);

private:
   [[maybe_unused]] gfx::render_manager& m_render_manager;

   util::dynamic_array<particle> m_particles;
   util::dynamic_array<std::string> m_names;
};
