#include <libowl/gfx/render_surface.hpp>

namespace owl::inline v0
{
   render_surface::render_surface(vk::UniqueSurfaceKHR&& surface) : m_surface(std::move(surface)) {}

   render_surface::operator vk::SurfaceKHR() const noexcept { return m_surface.get(); }
} // namespace owl::inline v0
