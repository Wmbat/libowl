#ifndef LIBOWL_GUI_EVENT_FOCUS_EVENT_HPP_
#define LIBOWL_GUI_EVENT_FOCUS_EVENT_HPP_

#include <libowl/types.hpp>

namespace owl::inline v0
{
   /**
    *
    */
   enum struct focus_type
   {
      in, ///< Signifies a window gaining focus
      out ///< Signifies a window loosing focus
   };

   struct focus_event
   {
      focus_type type; ///<
      u32 window_id;   ///<
   };
} // namespace owl::inline v0

#endif // LIBOWL_GUI_EVENT_FOCUS_EVENT_HPP_
