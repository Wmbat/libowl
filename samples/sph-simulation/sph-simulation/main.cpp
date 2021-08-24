#include <sph-simulation/simulation.hpp>

#include <sph-simulation/sim_config_parser.hpp>

#include <libutils/logger.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/span.hpp>
#include <range/v3/view/tail.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <cstdlib>

auto main(int argc, char** argv) -> int
{
   // clang-format off
   const std::vector arguments = ranges::span<char*>{argv, argc} 
      | ranges::views::tail
      | ranges::to<std::vector<std::string_view>>;
   // clang-format on

   auto logger = util::logger("sph-simulation");

   if (arguments.empty())
   {
      logger.error(
         "No command line arguments provided. Please provide path to scene settings (.json)");
      logger.error("Exiting...");

      return EXIT_FAILURE;
   }

   if (auto config = parse_sim_config_json(arguments[0]))
   {
      glfwInit();

      auto sim = simulation(settings{});
      sim.run();
   }
   else
   {
      return EXIT_FAILURE;
   }

   return EXIT_SUCCESS;
}
