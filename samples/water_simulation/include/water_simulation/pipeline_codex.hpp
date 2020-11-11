#pragma once

#include <water_simulation/core.hpp>
#include <water_simulation/render_system.hpp>

#include <gfx/render_pass.hpp>

#include <vkn/pipeline.hpp>

enum struct pipeline_codex_error
{

};

auto to_string(pipeline_codex_error err) -> std::string;
auto to_err_code(pipeline_codex_error err) -> util::error_t;

class pipeline_codex
{
   using graphics_map = std::unordered_map<std::string, vkn::graphics_pipeline>;

public:
   using key_type = std::string;
   using value_type = vkn::graphics_pipeline;

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
   pipeline_codex(render_system& renderer, std::shared_ptr<util::logger> p_logger);

   auto insert(const filepath& path, vkn::pipeline_type type) -> result<insert_kv>;
   auto lookup(const key_type& key) -> result<lookup_v>;
   auto remove(const key_type& key) -> result<remove_v>;

private:
   graphics_map m_graphics_pipeline;

   render_system& m_renderer;

   std::shared_ptr<util::logger> mp_logger;
};
