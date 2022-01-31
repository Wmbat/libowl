#include <sstream>
#include <stdexcept>

#include <libgerbil/assert.hpp>

void some_other_func(uint16_t flags, uint16_t mask)
{
   assert(mask bitand flags); // NOLINT
   assert(0xf == 16); // NOLINT

}

auto main() -> int
{
   int i = 0;

   assert(i == 10); // NOLINT
   assert(1, 1 bitand 2); // NOLINT
   assert(18446744073709551606ULL == -10); // NOLINT

   const uint16_t flags = 0b000101010;
   const uint16_t mask = 0b110010101;

   some_other_func(flags, mask);

   return 0;
}
