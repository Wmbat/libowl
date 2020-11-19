#pragma once

#include <water_simulation/core.hpp>

#include <glm/vec3.hpp>

namespace kernel
{
   auto poly6_constant(float kernel_radius) -> float;
   auto poly6_grad_constant(float kernel_radius) -> float;
   auto poly6(float kernel_radius, float r) -> float;
   auto poly6_grad(const glm::vec3& vec, float kernel_radius, float r) -> glm::vec3;

   auto spiky_constant(float kernel_radius) -> float;
   auto spiky_grad_constant(float kernel_radius) -> float;
   auto spiky(float kernel_radius, float r) -> float;
   auto spiky_grad(const glm::vec3& vec, float kernel_radius, float r) -> glm::vec3;

   auto viscosity_constant(float kernel_radius) -> float;
   auto viscosity(float kernel_radius, float r) -> float;

   auto cohesion_constant(float kernel_radius) -> float;
   auto cohesion(float kernel_radius, float r) -> float;
}; // namespace kernel
