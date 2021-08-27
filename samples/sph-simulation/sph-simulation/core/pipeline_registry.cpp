#include <sph-simulation/core/pipeline_registry.hpp>

#include <libreglisse/operations/transform_err.hpp>
#include <libreglisse/try.hpp>

#include <utility>

using namespace reglisse;

pipeline_registry::pipeline_registry(util::log_ptr logger) : m_logger{logger} {}

auto pipeline_registry::insert(graphics_pipeline_create_info&& info)
   -> reglisse::result<insert_kv, pipeline_registry_error>
{
   auto pipeline = graphics_pipeline{std::move(info)};
   const std::size_t key = id_counter++;

   if (auto [it, res] = m_graphics_pipelines.try_emplace(key, std::move(pipeline)); !res)
   {
      return err(pipeline_registry_error::failed_to_insert_pipeline);
   }

   return ok(insert_kv{key_type{key}, &m_graphics_pipelines.at(key)});
}
auto pipeline_registry::lookup(const key_type& key)
   -> reglisse::result<lookup_v, pipeline_registry_error>
{
   return try_wrap<std::exception>([&] {
             return lookup_v{&m_graphics_pipelines.at(key)};
          }) |
      transform_err([](const std::exception& /*err*/) {
             return pipeline_registry_error::pipeline_not_found;
          });
}
auto pipeline_registry::remove(const key_type& key)
   -> reglisse::result<remove_v, pipeline_registry_error>
{
   auto it = m_graphics_pipelines.find(key);
   if (it != std::end(m_graphics_pipelines))
   {
      remove_v res{std::move(it->second)};

      m_graphics_pipelines.erase(key);

      return ok(std::move(res));
   }

   return err(pipeline_registry_error::pipeline_not_found);
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

// LOOKUP_V

pipeline_registry::lookup_v::lookup_v(value_type* p_value) : mp_value{p_value} {}

auto pipeline_registry::lookup_v::value() const -> value_type&
{
   return *mp_value;
}

// INSERT_KV

pipeline_registry::insert_kv::insert_kv(key_type key, value_type* p_value) :
   m_key{key}, mp_value{p_value}
{}

auto pipeline_registry::insert_kv::key() const -> const key_type&
{
   return m_key;
}

auto pipeline_registry::insert_kv::value() const -> value_type&
{
   return *mp_value;
}

// REMOVE_V

pipeline_registry::remove_v::remove_v(value_type&& value) : m_value{std::move(value)} {}

auto pipeline_registry::remove_v::value() -> value_type&
{
   return m_value;
}

auto pipeline_registry::remove_v::take() -> value_type
{
   return std::move(m_value);
}
