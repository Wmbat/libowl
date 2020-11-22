#include <water_simulation/simulation.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

auto main() -> int
{
   settings settings;
   settings.time_step = 1 / 60.0f;

   glfwInit();

   simulation sim{settings};
   sim.run();

   return 0;
}
