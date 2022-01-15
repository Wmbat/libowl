#include <libowl/owl.hpp>

auto main() -> int
{
   std::string app_name = "gui-demo";

   auto gui_system = owl::system(app_name);
   [[maybe_unused]] auto& main_window = gui_system.make_window(app_name);

   return gui_system.run();
}
