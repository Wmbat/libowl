/**
 * @file libowl/gfx/render_surface.cpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date 22nd of January 2022
 * @brief 
 * @copyright Copyright (C) 2022 wmbat
 */

#include <libowl/gfx/render_surface.hpp>

namespace owl::inline v0
{
   render_surface::render_surface(vk::UniqueSurfaceKHR&& surface) : m_surface(std::move(surface)) {}

   render_surface::operator vk::SurfaceKHR() const noexcept { return m_surface.get(); }
} // namespace owl::inline v0
