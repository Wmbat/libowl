#ifndef SPH_SIMULATION_SCENE_PARSER_HPP
#define SPH_SIMULATION_SCENE_PARSER_HPP

#include <sph-simulation/sim_variables.hpp>

#include <libmannele/dimension.hpp>
#include <libmannele/error/runtime_error.hpp>

#include <libutils/logger.hpp>

#include <libreglisse/result.hpp>

#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

using namespace std::literals::chrono_literals;

template <typename Any, typename Ratio = std::ratio<1>>
using duration = std::chrono::duration<Any, Ratio>;

struct scene_data
{
   std::string name;

   mannele::dimension_u32 dimensions;
   mannele::u32 frame_count;

   duration<float, std::milli> time_step;

   sim_variables variables;
};

enum class scene_parse_error
{
   e_missing_marker_id,
   e_no_name_provided,
   e_dimension_field_error,
   e_no_frame_count_provided,
   e_no_time_step_provided,
   e_variables_field_error
};

auto make_error_condition(scene_parse_error e) -> std::error_condition;

auto open_file(const std::filesystem::path& filepath) -> std::string;

auto parse_scene_json(const std::filesystem::path& filepath)
   -> reglisse::result<scene_data, mannele::runtime_error>;

#endif // SPH_SIMULATION_SCENE_PARSER_HPP
