#ifndef LIBOWL_GFX_RENDER_SURFACE_HPP_
#define LIBOWL_GFX_RENDER_SURFACE_HPP_

#include <libash/instance.hpp>

namespace owl::inline v0
{
   class render_surface
   {
   public:
      render_surface() = default;
      render_surface(vk::UniqueSurfaceKHR&& surface);

      operator vk::SurfaceKHR() const noexcept;

   private:
      vk::UniqueSurfaceKHR m_surface;
   };
} // namespace owl::inline v0

#endif // LIBOWL_GFX_RENDER_SURFACE_HPP_
