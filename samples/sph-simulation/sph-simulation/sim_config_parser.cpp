#include <sph-simulation/sim_config_parser.hpp>

#include <sph-simulation/core/maths.hpp>

#include <libmannele/io/read_file.hpp>

#include <libreglisse/maybe.hpp>
#include <libreglisse/operations/and_then.hpp>

#include <nlohmann/json.hpp>

#include <magic_enum.hpp>

#include <fstream>

using namespace reglisse;

struct file_reading_error_category : std::error_category
{
   [[nodiscard]] auto name() const noexcept -> const char* override { return "scene_parser"; }
   [[nodiscard]] auto message(int err) const -> std::string override
   {
      return std::string(magic_enum::enum_name(static_cast<scene_parse_error>(err)));
   }
};

inline static const file_reading_error_category buffer_category{};

auto make_error_condition(scene_parse_error code) -> std::error_condition
{
   return std::error_condition({static_cast<int>(code), buffer_category});
}

auto extract_dimensions(const nlohmann::basic_json<>& dimensions)
   -> result<mannele::dimension_u32, mannele::runtime_error>
{
   const auto it_width = dimensions.find("width");
   if (it_width == std::end(dimensions))
   {
      return err(mannele::runtime_error(
         make_error_condition(scene_parse_error::e_dimension_field_error),
         R"(The "width" integer was not provided within the "dimensions" object)"));
   }

   const auto it_height = dimensions.find("height");
   if (it_height == std::end(dimensions))
   {
      return err(mannele::runtime_error(
         make_error_condition(scene_parse_error::e_dimension_field_error),
         R"(The "height" integer was not provided within the "dimensions" object)"));
   }

   return ok(mannele::dimension_u32{.width = *it_width, .height = *it_height});
}

auto extract_simulation_constants(const nlohmann::basic_json<>& variable)
   -> result<sim_variables, mannele::runtime_error>
{
   const auto err_cond = make_error_condition(scene_parse_error::e_variables_field_error);

   float gas_constant = 0.0f;
   if (auto it = variable.find("gas_contant"); it != std::end(variable))
   {
      if (it->is_number_float())
      {
         gas_constant = *it;
      }
      else
      {
         return err(mannele::runtime_error(err_cond, "The \"gas_constant\" field is not a float"));
      }
   }
   else
   {
      return err(mannele::runtime_error(err_cond, "The \"gas_constant\" field was not found"));
   }

   float rest_density = 0.0f;
   if (const auto it = variable.find("rest_density"); it != std::end(variable))
   {
      if (it->is_number_float())
      {
         rest_density = *it;
      }
      else
      {
         return err(mannele::runtime_error(err_cond, "The \"rest_density\" field is not a float"));
      }
   }
   else
   {
      return err(mannele::runtime_error(err_cond, "The \"rest_density\" field was not found"));
   }

   float viscosity_constant = 0.0f;
   if (const auto it = variable.find("viscosity_constant"); it != std::end(variable))
   {
      if (it->is_number_float())
      {
         viscosity_constant = *it;
      }
      else
      {
         return err(
            mannele::runtime_error(err_cond, "the \"viscosity_constant\" field is not a float"));
      }
   }
   else
   {
      return err(
         mannele::runtime_error(err_cond, "The \"viscosity_constant\" field was not found"));
   }

   float surface_tension_coefficient = 0.0f;
   if (const auto it = variable.find("surface_tension_coefficient"); it != std::end(variable))
   {
      if (it->is_number_float())
      {
         surface_tension_coefficient = *it;
      }
      else
      {
         return err(mannele::runtime_error(
            err_cond, "The \"surface_tension_coefficient\" field is not a float"));
      }
   }
   else
   {
      return err(mannele::runtime_error(err_cond,
                                        "The \"surface_tension_coefficient\" field was not found"));
   }

   float gravity_multiplier = 1.0f;
   if (const auto it = variable.find("gravity_multiplier"); it != std::end(variable))
   {
      if (it->is_number_float())
      {
         gravity_multiplier = *it;
      }
      else
      {
         return err(
            mannele::runtime_error(err_cond, "The \"gravity_multiplier\" field is not a float"));
      }
   }
   else
   {
      return err(
         mannele::runtime_error(err_cond, "The \"gravity_multiplier\" field was not found"));
   }

   float kernel_multiplier = 1.0f;
   if (const auto it = variable.find("kernel_multiplier"); it != std::end(variable))
   {
      if (it->is_number_float())
      {
         kernel_multiplier = *it;
      }
      else
      {
         return err(
            mannele::runtime_error(err_cond, "The \"kernel_multiplier\" field is not a float"));
      }
   }
   else
   {
      return err(mannele::runtime_error(err_cond, "The \"kernel_multiplier\" field was not found"));
   }

   float water_radius = 0.0f;
   if (auto it = variable.find("water_radius"); it != std::end(variable))
   {
      if (it->is_number_float())
      {
         water_radius = *it;
      }
      else
      {
         return err(mannele::runtime_error(err_cond, "The \"water_radius\" field is not a float"));
      }
   }
   else
   {
      return err(mannele::runtime_error(err_cond, "The \"water_radius\" field was not found"));
   }

   return ok(sim_variables{.gas_constant = gas_constant,
                           .rest_density = rest_density,
                           .viscosity_constant = viscosity_constant,
                           .surface_tension_coefficient = surface_tension_coefficient,
                           .gravity_multiplier = gravity_multiplier,
                           .kernel_multiplier = kernel_multiplier,
                           .water_radius = water_radius,
                           .water_mass = rest_density * cube(water_radius * 2)});
}

auto extract_rendering_data(const nlohmann::basic_json<>& rendering)
   -> result<std::pair<bool, bool>, mannele::runtime_error>
{
   const auto err_cond = make_error_condition(scene_parse_error::e_rendering_field_error);

   bool is_onscreen_enabled = false;
   if (const auto it = rendering.find("enable_onscreen"); it != std::end(rendering))
   {
      if (it->is_boolean())
      {
         is_onscreen_enabled = *it;
      }
      else
      {
         return err(
            mannele::runtime_error(err_cond, "The \"enable_onscreen\" field is not a bool"));
      }
   }
   else
   {
      is_onscreen_enabled = true;
   }

   bool is_offscreen_enabled = false;
   if (const auto it = rendering.find("enable_offscreen"); it != std::end(rendering))
   {
      if (it->is_boolean())
      {
         is_offscreen_enabled = *it;
      }
      else
      {
         return err(
            mannele::runtime_error(err_cond, "The \"enable_offscreen\" field is not a bool"));
      }
   }
   else
   {
      is_offscreen_enabled = true;
   }

   return ok(std::pair(is_onscreen_enabled, is_offscreen_enabled));
}

auto extract_scene_data(std::string&& str) -> result<sim_config, mannele::runtime_error>
{
   const auto sph = nlohmann::json::parse(str);

   const auto it_name = sph.find("name");
   if (it_name == std::end(sph))
   {
      return err(mannele::runtime_error(make_error_condition(scene_parse_error::e_no_name_provided),
                                        "The id \"name\" with it value string was not provided"));
   }

   const auto it_rendering = sph.find("rendering");
   if (it_rendering == std::end(sph))
   {
      return err(
         mannele::runtime_error(make_error_condition(scene_parse_error::e_rendering_field_error),
                                "The \"rendering\" object was not found"));
   }

   const auto it_dimensions = sph.find("dimensions");
   if (it_dimensions == std::end(sph))
   {
      return err(
         mannele::runtime_error(make_error_condition(scene_parse_error::e_dimension_field_error),
                                "The \"dimensions\" object was not found"));
   }

   const auto it_frame_count = sph.find("frame_count");
   if (it_frame_count == std::end(sph))
   {
      return err(
         mannele::runtime_error(make_error_condition(scene_parse_error::e_framecount_field_error),
                                "The id \"frame_count\" with it's integer value was not provided"));
   }

   const auto it_time_step = sph.find("time_step");
   if (it_time_step == std::end(sph))
   {
      return err(
         mannele::runtime_error(make_error_condition(scene_parse_error::e_time_step_field_error),
                                "The id \"time_step\" with it's integer value was not provided"));
   }

   const auto it_variables = sph.find("variables");
   if (it_variables == std::end(sph))
   {
      return err(
         mannele::runtime_error(make_error_condition(scene_parse_error::e_variables_field_error),
                                "The \"variables\" object was not found"));
   }

   sim_config data;
   data.name = *it_name;
   data.frame_count = *it_frame_count;
   data.time_step = std::chrono::duration<float, std::milli>(*it_time_step);

   if (auto rendering = extract_rendering_data(*it_rendering))
   {
      data.is_onscreen_rendering_enabled = rendering.borrow().first;
      data.is_offscreen_rendering_enabled = rendering.borrow().second;
   }
   else
   {
      return err(rendering.borrow_err());
   }

   if (auto dimensions = extract_dimensions(*it_dimensions))
   {
      data.dimensions = dimensions.borrow();
   }
   else
   {
      return err(dimensions.borrow_err());
   }

   if (auto constants = extract_simulation_constants(*it_variables))
   {
      data.variables = constants.borrow();
   }
   else
   {
      return err(constants.borrow_err());
   }

   return ok(data);
}

auto parse_sim_config_json(const std::filesystem::path& filepath)
   -> result<sim_config, mannele::runtime_error>
{
   // clang-format on

   return mannele::unbuffered_file_read(filepath) | and_then(extract_scene_data);

   // clang-format off
}
