#pragma once

#include <water_simulation/render_system.hpp>
#include <water_simulation/renderable.hpp>

#include <util/containers/dynamic_array.hpp>

#include <glm/vec3.hpp>

#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

static constexpr std::size_t default_particle_count = 1000U;

struct particle_prop
{
   glm::vec3 position{};
   glm::vec3 velocity{};
   glm::vec3 force{};

   float density{0.0F};
   float pressure{0.0F};
   float mass{1.0F};
};

class particle_engine
{
public:
   struct particle
   {
      glm::vec3 position{};
      glm::vec3 velocity{};
      glm::vec3 force{};

      float mass{1.0F};
      float density{0.0F};
      float pressure{0.0F};

      bool is_active{false};

      glm::mat4 model;
   };

public:
   particle_engine(std::shared_ptr<util::logger> p_logger);

   void emit(const particle_prop& particle);

   [[nodiscard]] inline auto particles() -> std::span<particle>
   {
      // clang-format off
      return m_particles;
      // clang-format on
   }

   [[nodiscard]] inline auto particle_models() const
   {
      // clang-format off
      return m_particles 
         | ranges::views::filter([](auto p) { return p.is_active; }) 
         | ranges::views::transform([](particle p) { return p.model; });
      // clang-format on
   }

private:
   util::dynamic_array<particle> m_particles;

   std::shared_ptr<util::logger> mp_logger;
};
