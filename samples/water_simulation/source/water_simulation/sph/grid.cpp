#include <water_simulation/sph/grid.hpp>

namespace sph
{
   grid::grid(float cell_size, const glm::vec3& center, const glm::vec3& dimensions) :
      m_cell_size{cell_size}, m_center{center}, m_dimensions{dimensions}
   {}
} // namespace sph
