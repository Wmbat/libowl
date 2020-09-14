#include <gfx/render_pass.hpp>

#include <gfx/render_manager.hpp>

namespace gfx
{
   render_pass::render_pass(render_manager* p_render_manager) : mp_render_manager{p_render_manager}
   {}
} // namespace gfx
