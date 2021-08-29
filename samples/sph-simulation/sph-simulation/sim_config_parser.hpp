#ifndef SPH_SIMULATION_SIM_CONFIG_PARSER_HPP
#define SPH_SIMULATION_SIM_CONFIG_PARSER_HPP

#include <sph-simulation/sim_config.hpp>

#include <libmannele/dimension.hpp>
#include <libmannele/error/runtime_error.hpp>

#include <libutils/logger.hpp>

#include <libreglisse/result.hpp>

#include <filesystem>
#include <string>
#include <vector>

enum class scene_parse_error
{
   e_no_name_provided,
   e_dimension_field_error,
   e_rendering_field_error,
   e_framecount_field_error,
   e_time_step_field_error,
   e_variables_field_error
};

auto make_error_condition(scene_parse_error e) -> std::error_condition;

auto open_file(const std::filesystem::path& filepath) -> std::string;

auto parse_sim_config_json(const std::filesystem::path& filepath)
   -> reglisse::result<sim_config, mannele::runtime_error>;

#endif // SPH_SIMULATION_SIM_CONFIG_PARSER_HPP
