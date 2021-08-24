#include <sph-simulation/scene_parser.hpp>

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

auto parse_json(std::string&& data) -> result<nlohmann::basic_json<>, mannele::runtime_error>
{
   const auto json = nlohmann::json::parse(data);

   if (!json.contains("sph"))
   {
      return err(
         mannele::runtime_error(make_error_condition(scene_parse_error::e_missing_marker_id),
                                "Failed to find \"sph\" designator in json file"));
   }

   return ok(json["sph"]);
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

auto extract_simulation_constants(const nlohmann::basic_json<>& constants)
   -> result<sim_variables, mannele::runtime_error>
{}

auto extract_scene_data(nlohmann::basic_json<>&& sph) -> result<scene_data, mannele::runtime_error>
{
   const auto it_name = sph.find("name");
   if (it_name == std::end(sph))
   {
      return err(mannele::runtime_error(make_error_condition(scene_parse_error::e_no_name_provided),
                                        "The id \"name\" with it value string was not provided"));
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
         mannele::runtime_error(make_error_condition(scene_parse_error::e_no_frame_count_provided),
                                "The id \"frame_count\" with it's integer value was not provided"));
   }

   const auto it_time_step = sph.find("time_step");
   if (it_time_step == std::end(sph))
   {
      return err(
         mannele::runtime_error(make_error_condition(scene_parse_error::e_no_time_step_provided),
                                "The id \"time_step\" with it's integer value was not provided"));
   }

   const auto it_constants = sph.find("constants");
   if (it_constants == std::end(sph))
   {
      return err(
         mannele::runtime_error(make_error_condition(scene_parse_error::e_variables_field_error),
                                "The \"constants\" object was not found"));
   }

   scene_data data;
   data.name = *it_name;
   data.frame_count = *it_frame_count;
   data.time_step = duration<float, std::milli>(*it_time_step);

   if (auto dimensions = extract_dimensions(*it_dimensions))
   {
      data.dimensions = dimensions.borrow();
   }
   else
   {
      return err(dimensions.borrow_err());
   }

   if (auto constants = extract_simulation_constants(*it_constants))
   {
      data.variables = constants.borrow();
   }
   else
   {
      return err(constants.borrow_err());
   }

   return ok(data);
}

auto parse_scene_json(const std::filesystem::path& filepath)
   -> result<scene_data, mannele::runtime_error>
{
   // clang-format on

   return mannele::unbuffered_file_read(filepath) | and_then(parse_json) |
      and_then(extract_scene_data);

   // clang-format off
}
