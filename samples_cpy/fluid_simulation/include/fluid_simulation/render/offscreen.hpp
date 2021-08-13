#pragma once

#include <fluid_simulation/core.hpp>
#include <fluid_simulation/render/camera.hpp>
#include <fluid_simulation/render/render_pass.hpp>

#include <cacao/gfx/image.hpp>
#include <cacao/vulkan/command_pool.hpp>

struct offscreen
{
   vkn::command_pool command_pool;

   vk::UniqueFence in_flight_fence;
   vk::UniqueSemaphore render_finished_semaphore;
   vk::UniqueSemaphore image_available_semaphore;

   cacao::image colour;
   cacao::image depth;
   render_pass pass;

   camera cam;

   cacao::vulkan::buffer image_buffer;
};
