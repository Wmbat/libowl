#ifndef SPH_SIMULATION_RENDER_I_FRAME_MANAGER_HPP
#define SPH_SIMULATION_RENDER_I_FRAME_MANAGER_HPP

#include <libcacao/command_pool.hpp>

#include <libmannele/core.hpp>

#include <libreglisse/maybe.hpp>

#include <span>

class i_frame_manager
{
public:
   i_frame_manager() = default;
   i_frame_manager(const i_frame_manager&) = default;
   i_frame_manager(i_frame_manager&&) = default;
   virtual ~i_frame_manager() = default;

   auto operator=(const i_frame_manager&) -> i_frame_manager& = default;
   auto operator=(i_frame_manager&&) -> i_frame_manager& = default;

   virtual auto begin_frame(std::span<cacao::command_pool> pools)
      -> reglisse::maybe<mannele::u32> = 0;
   virtual void end_frame(std::span<cacao::command_pool> pools) = 0;
};

#endif // SPH_SIMULATION_RENDER_I_FRAME_MANAGER_HPP
