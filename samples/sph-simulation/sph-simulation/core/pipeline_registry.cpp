#include <sph-simulation/core/pipeline_registry.hpp>

#include <libreglisse/operations/transform_err.hpp>
#include <libreglisse/try.hpp>

#include <utility>

using namespace reglisse;

pipeline_registry::pipeline_registry(mannele::log_ptr logger) : m_logger{logger} {}

auto pipeline_registry::insert(graphics_pipeline_create_info&& info)
   -> reglisse::result<insert_kv<pipeline_type::graphics>, pipeline_registry_error>
{
   auto gfx = pipeline<pipeline_type::graphics>(std::move(info));
   const std::size_t key = id_counter++;

   if (auto [it, res] = m_graphics_pipelines.try_emplace(key, std::move(gfx)); !res)
   {
      return err(pipeline_registry_error::failed_to_insert_pipeline);
   }

   return ok(insert_kv{key_type{key}, &m_graphics_pipelines.at(key)});
}

struct pipeline_registry_error_category : std::error_category
{
   [[nodiscard]] auto name() const noexcept -> const char* override { return "pipeline_registry"; }
   [[nodiscard]] auto message(int err) const -> std::string override
   {
      return std::string(magic_enum::enum_name(static_cast<pipeline_registry_error>(err)));
   }
};

static const pipeline_registry_error_category pipeline_codex_error_cat{};

auto to_string(pipeline_registry_error err) -> std::string
{
   if (err == pipeline_registry_error::pipeline_not_found)
   {
      return "pipeline_not_found";
   }

   return "UNKNOWN";
}
auto make_error_condition(pipeline_registry_error err) -> std::error_condition
{
   return std::error_condition({static_cast<int>(err), pipeline_codex_error_cat});
}
