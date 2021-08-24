#include <sph-simulation/render/shader_registry.hpp>

#include <libreglisse/operations/transform_err.hpp>
#include <libreglisse/try.hpp>

#include <magic_enum.hpp>

#include <fstream>

using namespace reglisse;

shader_registry::shader_registry(render_system& renderer, util::log_ptr logger) :
   m_renderer{renderer}, m_logger{logger}
{}

auto shader_registry::insert(const filepath& path, cacao::shader_type type)
   -> result<insert_kv, shader_registry_error>
{
   using file_it = std::istreambuf_iterator<char>;

   std::ifstream file{asset_default_dir / path, std::ios::binary};
   if (!file.is_open())
   {
      return err(shader_registry_error::failed_to_open_file);
   }

   const std::vector<uint8_t> raw_shader_data{file_it{file}, file_it{}};

   spirv_binary data{};
   data.resize(raw_shader_data.size() / sizeof(std::uint32_t));

   std::memcpy(static_cast<void*>(data.data()), raw_shader_data.data(),
               sizeof(std::uint32_t) * data.size());

   const auto [it, res] = m_shaders.try_emplace(path.string(),
                                                cacao::shader({.device = m_renderer.device(),
                                                               .name = path.string(),
                                                               .type = type,
                                                               .binary = data,
                                                               .logger = m_logger}));
   if (res)
   {
      return ok(insert_kv(it->first, &it->second));
   }

   return err(shader_registry_error::failed_to_insert_shader);
}

auto shader_registry::lookup(const key_type& key) -> result<lookup_v, shader_registry_error>
{
   return try_wrap<std::exception>([&] {
             return lookup_v{&m_shaders.at(key)};
          }) |
      transform_err([](const std::exception& /*err*/) {
             return shader_registry_error::shader_not_found;
          });
}

auto shader_registry::remove(const key_type& key) -> result<remove_v, shader_registry_error>
{
   auto it = m_shaders.find(key);
   if (it != std::end(m_shaders))
   {
      remove_v res{std::move(it->second)};

      m_shaders.erase(key);

      return ok(std::move(res));
   }

   return err(shader_registry_error::shader_not_found);
}

// LOOKUP_V

shader_registry::lookup_v::lookup_v(value_type* p_value) : mp_value{p_value} {}

auto shader_registry::lookup_v::value() const -> value_type&
{
   return *mp_value;
}

shader_registry::insert_kv::insert_kv(key_type key, value_type* p_value) :
   m_key{std::move(key)}, mp_value{p_value}
{}

auto shader_registry::insert_kv::key() const -> const key_type&
{
   return m_key;
}

auto shader_registry::insert_kv::value() const -> value_type&
{
   return *mp_value;
}

// REMOVE_V

shader_registry::remove_v::remove_v(value_type&& value) : m_value{std::move(value)} {}

auto shader_registry::remove_v::value() -> value_type&
{
   return m_value;
}

auto shader_registry::remove_v::take() -> value_type
{
   return std::move(m_value);
}

struct shader_registry_error_category : std::error_category
{
   [[nodiscard]] auto name() const noexcept -> const char* override { return "shader_registry"; }
   [[nodiscard]] auto message(int err) const -> std::string override
   {
      return std::string(magic_enum::enum_name(static_cast<shader_registry_error>(err)));
   }
};

static const shader_registry_error_category shader_codex_error_cat{};

auto make_error_condition(shader_registry_error err) -> std::error_condition
{
   return std::error_condition({static_cast<int>(err), shader_codex_error_cat});
}
