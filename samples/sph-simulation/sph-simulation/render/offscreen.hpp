#pragma once

#include <sph-simulation/core.hpp>
#include <sph-simulation/render/camera.hpp>
#include <sph-simulation/render/render_pass.hpp>

#include <libcacao/command_pool.hpp>

struct offscreen
{
   cacao::command_pool command_pool;

   vk::UniqueFence in_flight_fence;
   vk::UniqueSemaphore render_finished_semaphore;
   vk::UniqueSemaphore image_available_semaphore;

   image colour;
   image depth;
   render_pass pass;

   camera cam;

   cacao::buffer image_buffer;
};
