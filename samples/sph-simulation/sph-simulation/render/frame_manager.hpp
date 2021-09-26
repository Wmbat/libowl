#ifndef SPH_SIMULATION_RENDER_FRAME_MANAGER_HPP
#define SPH_SIMULATION_RENDER_FRAME_MANAGER_HPP

#include <sph-simulation/render/core/framebuffer.hpp>
#include <sph-simulation/render/core/image.hpp>

#include <libcacao/command_pool.hpp>
#include <libcacao/swapchain.hpp>
#include <libcacao/window.hpp>

#include <libmannele/core.hpp>
#include <libmannele/logging/log_ptr.hpp>

#include <libreglisse/maybe.hpp>

static constexpr std::size_t max_frames_in_flight = 2;
static constexpr std::size_t expected_image_count = 3;

struct frame_manager_create_info
{
   cacao::window& window;
   cacao::device& device;
   vk::SurfaceKHR surface;

   vk::ImageUsageFlags image_usage; 

   mannele::log_ptr logger;
};

struct frame_data
{
   mannele::u32 image_index;
   mannele::u32 frame_index;
};

class frame_manager
{
public:
   frame_manager() = default;
   frame_manager(const frame_manager_create_info& info);

   auto begin_frame() -> reglisse::maybe<frame_data>;
   void end_frame(std::span<cacao::command_pool> pools);

   [[nodiscard]] auto frame_format() const noexcept -> vk::Format;
   [[nodiscard]] auto extent() const noexcept -> const vk::Extent2D;
   [[nodiscard]] auto image_count() const noexcept -> mannele::u64;

   [[nodiscard]] auto get_framebuffer_info() const -> std::vector<framebuffer_create_info>;

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

#endif // SPH_SIMULATION_RENDER_FRAME_MANAGER_HPP
