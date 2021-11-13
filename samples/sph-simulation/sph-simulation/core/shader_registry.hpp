#pragma once

#include <sph-simulation/core.hpp>

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
   shader_registry(cacao::device& device, mannele::log_ptr logger);

   auto insert(const filepath& path, cacao::shader_type type)
      -> reglisse::result<insert_kv, shader_registry_error>;
   auto lookup(const key_type& key) -> reglisse::result<lookup_v, shader_registry_error>;
   auto remove(const key_type& key) -> reglisse::result<remove_v, shader_registry_error>;

private:
   shader_map m_shaders;

   cacao::device& m_device;

   mannele::log_ptr m_logger;
};
