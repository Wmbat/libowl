#pragma once

#include <sph-simulation/core.hpp>
#include <sph-simulation/core/pipeline.hpp>
#include <sph-simulation/render/render_system.hpp>

enum struct pipeline_registry_error
{
   failed_to_insert_pipeline,
   pipeline_not_found
};

auto make_error_condition(pipeline_registry_error err) -> std::error_condition;

class pipeline_registry
{
   using graphics_map = std::unordered_map<std::size_t, graphics_pipeline>;

public:
   using key_type = mannele::u64;
   using value_type = graphics_pipeline;

   class lookup_v
   {
   public:
      lookup_v(value_type* p_value);

      [[nodiscard]] auto value() const -> value_type&;

   private:
      value_type* mp_value;
   };

   class insert_kv
   {
   public:
      insert_kv(key_type key, value_type* value);

      [[nodiscard]] auto key() const -> const key_type&;
      [[nodiscard]] auto value() const -> value_type&;

   private:
      key_type m_key;
      value_type* mp_value;
   };

   class remove_v
   {
   public:
      remove_v(value_type&& value);

      [[nodiscard]] auto value() -> value_type&;

      auto take() -> value_type;

   private:
      value_type m_value;
   };

public:
   pipeline_registry(mannele::log_ptr logger);

   auto insert(graphics_pipeline_create_info&& info)
      -> reglisse::result<insert_kv, pipeline_registry_error>;
   auto lookup(const key_type& key) -> reglisse::result<lookup_v, pipeline_registry_error>;
   auto remove(const key_type& key) -> reglisse::result<remove_v, pipeline_registry_error>;

private:
   graphics_map m_graphics_pipelines;

   mannele::log_ptr m_logger;

   mannele::u64 id_counter{0};
};
