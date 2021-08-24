#include <sph-simulation/physics/kernel.hpp>

namespace kernel
{
   auto poly6_constant(float kernel_radius) -> float
   {
      return 315.0F / (65.0F * pi * std::pow(kernel_radius, 9.0F)); // NOLINT
   }
   auto poly6_grad_constant(float kernel_radius) -> float
   {
      return -945.f / (32.f * pi * std::pow(kernel_radius, 9.f)); // NOLINT
   }
   auto poly6(float h2, float r2) -> float { return cube(h2 - r2); }
   auto poly6_grad(const glm::vec3& vec, float h2, float r2) -> glm::vec3
   {
      return square(h2 - r2) * vec;
   }

   auto spiky_constant(float kernel_radius) -> float
   {
      return 15.0f / (pi * my_pow(kernel_radius, 6u)); // NOLINT
   }
   auto spiky_grad_constant(float kernel_radius) -> float
   {
      return -45.0f / (pi * my_pow(kernel_radius, 6u)); // NOLINT
   }
   auto spiky(float kernel_radius, float r) -> float { return square(kernel_radius - r); }
   auto spiky_grad(const glm::vec3& vec, float kernel_radius, float r) -> glm::vec3
   {
      return vec * (square(kernel_radius - r) * (1.0f / r));
   }

   auto viscosity_constant(float kernel_radius) -> float
   {
      return 45.0F / (pi * std::pow(kernel_radius, 6.0f)); // NOLINT
   }
   auto viscosity(float kernel_radius, float r) -> float { return kernel_radius - r; }

   auto cohesion_constant(float kernel_radius) -> float
   {
      return 32.0f / (pi * std::pow(kernel_radius, 9.0f)); // NOLINT
   }
   auto cohesion(float kernel_radius, float r) -> float
   {
      if (r <= kernel_radius / 2.0f) // NOLINT
      {
         const float offset = std::pow(kernel_radius, 6.0f) / 64.0f;

         return 2.0f * cube(kernel_radius - r) * cube(r) - offset; // NOLINT
      }

      return cube(kernel_radius - r) * cube(r);
   }
} // namespace kernel
