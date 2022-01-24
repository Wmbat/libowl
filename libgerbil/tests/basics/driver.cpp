#include <sstream>
#include <stdexcept>

#include <libgerbil/assert.hpp>

auto main () -> int
{
   assert(nullptr); // NOLINT

   return 0;
}
