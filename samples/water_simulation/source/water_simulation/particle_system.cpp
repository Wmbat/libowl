#include <utility>
#include <water_simulation/particle_system.hpp>

#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include <glm/ext/matrix_transform.hpp>

#include <utility>

particle_engine::particle_engine(std::shared_ptr<util::logger> p_logger) :
   mp_logger{std::move(p_logger)}
{}

void particle_engine::emit(const particle_prop& prop)
{
   m_particles.emplace_back(particle{.position = prop.position,
                                     .velocity = prop.velocity,
                                     .force = prop.force,
                                     .mass = prop.mass,
                                     .density = prop.density,
                                     .pressure = prop.pressure,
                                     .is_active = true,
                                     .model = glm::mat4{1}});
}
