#pragma once

namespace gfx
{
   class render_manager;

   class render_pass
   {
   public:
      render_pass(render_manager* mp_render_manager);

   private:
      render_manager* mp_render_manager;
   };
} // namespace gfx
