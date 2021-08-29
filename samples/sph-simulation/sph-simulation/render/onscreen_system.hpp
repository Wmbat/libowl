#ifndef SPH_SIMULATION_RENDER_ONSCREEN_RENDERER_HPP
#define SPH_SIMULATION_RENDER_ONSCREEN_RENDERER_HPP

#include <sph-simulation/render/core/image.hpp>

#include <libcacao/command_pool.hpp>
#include <libcacao/swapchain.hpp>
#include <libcacao/window.hpp>

static constexpr std::size_t max_frames_in_flight = 2;
static constexpr std::size_t expected_image_count = 3;

class onscreen_renderer
{
public:
private:
   cacao::window m_window;

   cacao::swapchain m_swapchain;

   std::vector<vk::UniqueSemaphore> m_render_finished_semaphores;

   std::array<cacao::command_pool, max_frames_in_flight> m_render_command_pools;
   std::array<vk::UniqueSemaphore, max_frames_in_flight> m_image_available_semaphores;
   std::array<vk::UniqueFence, max_frames_in_flight> m_in_flight_fences;

   std::vector<vk::Fence> m_images_in_flight{};

   image m_depth_image{};

   mannele::u32 m_current_image_index{};
   mannele::u32 m_current_frame_index{};
};

#endif // SPH_SIMULATION_RENDER_ONSCREEN_RENDERER_HPP
