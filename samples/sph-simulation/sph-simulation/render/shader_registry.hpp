#pragma once

#include <sph-simulation/core.hpp>
#include <sph-simulation/render/render_system.hpp>

#include <libcacao/shader.hpp>

#include <filesystem>
#include <string>

enum struct shader_registry_error
{
   failed_to_open_file,
   failed_to_insert_shader,
   shader_not_found
};

auto make_error_condition(shader_registry_error err) -> std::error_condition;

using spirv_binary = std::vector<mannele::u32>;

class shader_registry
{
   using shader_map = std::unordered_map<std::string, cacao::shader>;

public:
   using key_type = std::string;
   using value_type = cacao::shader;

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
   shader_registry(render_system& renderer, util::log_ptr logger);

   /**
    * @brief Insert and construct a `vkn::shader` into the registry.
    *
    * @param path The path to the spirv shader binary
    * @param type The type of shader to insert
    *
    * @return A `result` holding one of two things:
    * * A `util::error_t` holding relevant information about the error that happened during insert.
    * * A struct giving access to the `key_type` & `value_type` that have been inserted into the
    * registry.
    */
   auto insert(const filepath& path, cacao::shader_type type)
      -> reglisse::result<insert_kv, shader_registry_error>;
   /**
    * @brief Finds a `value_type` from it's associated `key_type`
    *
    * @param key The key used for finding the associated `value_type`
    *
    * @return A `result` holding one of two things:
    * * A `util::error_t` holding relevant information about the error that happened during lookup.
    * * A struct giving access to `value_type` object stored within the registry.
    */
   auto lookup(const key_type& key) -> reglisse::result<lookup_v, shader_registry_error>;
   /**
    * @brief Remove a `value_type` from the registry using it's associated `key_type`
    *
    * @param key The key used for finding the associated `value_type`
    *
    * @return A `result` holding one of two things:
    * * A `util::error_t` holding relevant information about the error that happened during removal.
    * * A struct giving ownership of the `value_type` object stored within the registry.
    */
   auto remove(const key_type& key) -> reglisse::result<remove_v, shader_registry_error>;

private:
   shader_map m_shaders;

   render_system& m_renderer;

   util::log_ptr m_logger;
};
