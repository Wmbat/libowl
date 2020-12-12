#pragma once

#include <water_simulation/core.hpp>
#include <water_simulation/render/camera.hpp>
#include <water_simulation/render/image.hpp>
#include <water_simulation/render/render_pass.hpp>

#include <vkn/command_pool.hpp>

struct offscreen
{
   vkn::command_pool command_pool;

   vkn::fence in_flight_fence;
   vkn::semaphore render_finished_semaphore;
   vkn::semaphore image_available_semaphore;

   image<image_flags::colour | image_flags::transfer_src> colour;
   image<image_flags::depth_stencil> depth;
   render_pass render_pass;

   camera camera;

   vkn::buffer image_buffer;
};
