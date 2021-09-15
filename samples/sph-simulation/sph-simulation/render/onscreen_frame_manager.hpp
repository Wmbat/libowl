#ifndef SPH_SIMULATION_RENDER_ONSCREEN_FRAME_MANAGER_HPP
#define SPH_SIMULATION_RENDER_ONSCREEN_FRAME_MANAGER_HPP

#include <sph-simulation/render/core/image.hpp>
#include <sph-simulation/render/i_frame_manager.hpp>

#include <libcacao/command_pool.hpp>
#include <libcacao/swapchain.hpp>
#include <libcacao/window.hpp>

#include <libmannele/core.hpp>
#include <libmannele/logging/log_ptr.hpp>

#include <libreglisse/maybe.hpp>

static constexpr std::size_t max_frames_in_flight = 2;
static constexpr std::size_t expected_image_count = 3;

struct onscreen_frame_manager_create_info
{
   cacao::window& window;
   cacao::device& device;
   vk::SurfaceKHR surface;

   mannele::log_ptr logger;
};

class onscreen_frame_manager : public i_frame_manager
{
public:
   onscreen_frame_manager() = default;
   onscreen_frame_manager(const onscreen_frame_manager_create_info& info);

   auto begin_frame(std::span<cacao::command_pool> pools) -> reglisse::maybe<mannele::u32> override;
   void end_frame(std::span<cacao::command_pool> pools) override;

private:
   mannele::log_ptr m_logger;

   cacao::device* mp_device = nullptr;
   cacao::swapchain m_swapchain;

   std::vector<vk::UniqueSemaphore> m_render_finished_semaphores;

   std::array<vk::UniqueSemaphore, max_frames_in_flight> m_image_available_semaphores;
   std::array<vk::UniqueFence, max_frames_in_flight> m_in_flight_fences;

   std::vector<vk::Fence> m_images_in_flight{};

   image m_depth_image{};

   mannele::u32 m_current_image_index{};
   mannele::u32 m_current_frame_index{};
};

#endif // SPH_SIMULATION_RENDER_ONSCREEN_FRAME_MANAGER_HPP
