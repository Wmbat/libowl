#include <water_simulation/simulation.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <easy/profiler.h>

auto main() -> int
{
   EASY_PROFILER_ENABLE

   settings settings;

   glfwInit();

   simulation sim{settings};
   sim.run();

   profiler::dumpBlocksToFile("profiling.prof");

   return 0;
}
