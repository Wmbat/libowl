#include <libowl/owl.hpp>

auto main() -> int
{
   auto gui_system = owl::system("gui-demo");
   [[maybe_unused]] auto& window_1 = gui_system.make_window("window 1");



//   auto& window_2 = gui_system.make_window("window 2");

   return gui_system.run();
}
