#include <libowl/owl.hpp>

auto main() -> int
{
   auto gui_system = owl::system("gui-demo");
   auto& window_1 = gui_system.make_window("window 1");

   return gui_system.run();
}
