#pragma once

#include <iosfwd>
#include <string>

#include <libowl/export.hpp>

namespace owl
{
  // Print a greeting for the specified name into the specified
  // stream. Throw std::invalid_argument if the name is empty.
  //
  LIBOWL_SYMEXPORT void
  say_hello (std::ostream&, const std::string& name);
}
