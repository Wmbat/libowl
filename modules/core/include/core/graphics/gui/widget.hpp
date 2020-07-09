#pragma once

#include <concepts>

namespace core::gfx
{
   class widget
   {
   public:
      widget() = default;

      widget& setParent(widget* p_parent_in);
      widget* getParent();

   private:
      widget* p_parent{nullptr};
   };
} // namespace core::gfx
