#include <fluid_simulation/simulation.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

auto main() -> int
{
   settings settings;

   glfwInit();

   auto sim = simulation(settings);
   sim.run();

   return 0;
}
