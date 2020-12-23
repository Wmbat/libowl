#pragma once

#include <fluid_simulation/core.hpp>
#include <fluid_simulation/render/camera.hpp>
#include <fluid_simulation/render/image.hpp>
#include <fluid_simulation/render/render_pass.hpp>

#include <vermillon/vulkan/command_pool.hpp>

struct offscreen
{
   vkn::command_pool command_pool;

   vkn::fence in_flight_fence;
   vkn::semaphore render_finished_semaphore;
   vkn::semaphore image_available_semaphore;

   image<image_flags::colour | image_flags::transfer_src> colour;
   image<image_flags::depth_stencil> depth;
   render_pass pass;

   camera cam;

   vkn::buffer image_buffer;
};
