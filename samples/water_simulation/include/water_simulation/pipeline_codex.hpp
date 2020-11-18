#pragma once

#include <water_simulation/core.hpp>
#include <water_simulation/pipeline.hpp>
#include <water_simulation/render_system.hpp>

enum struct pipeline_codex_error
{
   failed_to_insert_pipeline,
   pipeline_not_found
};

auto to_string(pipeline_codex_error err) -> std::string;
auto to_err_code(pipeline_codex_error err) -> util::error_t;

using pipeline_index_t =
   util::strong_type<std::size_t, struct pipeline_index_tag, util::arithmetic>;

class pipeline_codex
{
   using graphics_map = std::unordered_map<std::size_t, graphics_pipeline>;

public:
   using key_type = pipeline_index_t;
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
   pipeline_codex(std::shared_ptr<util::logger> p_logger);

   auto insert(graphics_pipeline::create_info&& info) -> result<insert_kv>;
   auto lookup(const key_type& key) -> result<lookup_v>;
   auto remove(const key_type& key) -> result<remove_v>;

private:
   graphics_map m_graphics_pipelines;

   std::shared_ptr<util::logger> mp_logger;

   std::size_t id_counter{0};
};
