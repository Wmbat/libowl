#include <water_simulation/shader_codex.hpp>

#include <monads/try.hpp>

#include <fstream>

auto to_util_err(vkn::error_t&& err) -> util::error_t
{
   return {err.value()};
};

shader_codex::shader_codex(gfx::render_manager& renderer, std::shared_ptr<util::logger> p_logger) :
   m_renderer{renderer}, mp_logger{std::move(p_logger)}
{}

auto shader_codex::insert(const filepath& path, vkn::shader_type type) -> result<insert_kv>
{
   using file_it = std::istreambuf_iterator<char>;

   std::ifstream file{path, std::ios::binary};
   if (!file.is_open())
   {
      return monad::err(to_err_code(shader_codex_error::failed_to_open_file));
   }

   const std::vector<uint8_t> raw_shader_data{file_it{file}, file_it{}};

   spirv_binary data(raw_shader_data.size() / sizeof(std::uint32_t));

   std::memcpy(static_cast<void*>(data.data()), raw_shader_data.data(),
               sizeof(std::uint32_t) * data.size());

   return vkn::shader::builder(m_renderer.device(), mp_logger)
      .set_name(path.string())
      .set_spirv_binary(data)
      .set_type(type)
      .build()
      .map_error(to_util_err)
      .and_then([&](vkn::shader&& shader) -> result<insert_kv> {
         const auto key = path.string();

         if (auto [it, res] = m_shaders.try_emplace(key, std::move(shader)); !res)
         {
            return monad::err(to_err_code(shader_codex_error::failed_to_insert_shader));
         }

         return insert_kv{key, &m_shaders.at(key)};
      });
}

auto shader_codex::lookup(const key_type& key) -> result<lookup_v>
{
   return monad::try_wrap<std::exception>([&] {
             return lookup_v{&m_shaders.at(key)};
          })
      .map_error([](const std::exception& /*err*/) {
         return to_err_code(shader_codex_error::shader_not_found);
      });
}

auto shader_codex::remove(const key_type& key) -> result<remove_v>
{
   auto it = m_shaders.find(key);
   if (it != std::end(m_shaders))
   {
      remove_v res{std::move(it->second)};

      m_shaders.erase(key);

      return res;
   }
   else // NOLINT
   {
      return monad::err(to_err_code(shader_codex_error::shader_not_found));
   }
}

// LOOKUP_V

shader_codex::lookup_v::lookup_v(value_type* p_value) : mp_value{p_value} {}

auto shader_codex::lookup_v::value() const -> value_type&
{
   return *mp_value;
}

shader_codex::insert_kv::insert_kv(key_type key, value_type* p_value) :
   m_key{std::move(key)}, mp_value{p_value}
{}

auto shader_codex::insert_kv::key() const -> const key_type&
{
   return m_key;
}

auto shader_codex::insert_kv::value() const -> value_type&
{
   return *mp_value;
}

// REMOVE_V

shader_codex::remove_v::remove_v(value_type&& value) : m_value{std::move(value)} {}

auto shader_codex::remove_v::value() -> value_type&
{
   return m_value;
}

auto shader_codex::remove_v::take() -> value_type
{
   return std::move(m_value);
}

struct shader_codex_error_category : std::error_category
{
   [[nodiscard]] auto name() const noexcept -> const char* override { return "shader_codex"; }
   [[nodiscard]] auto message(int err) const -> std::string override
   {
      return to_string(static_cast<shader_codex_error>(err));
   }
};

static const shader_codex_error_category shader_codex_error_cat{};

auto to_string(shader_codex_error err) -> std::string
{
   if (err == shader_codex_error::shader_not_found)
   {
      return "shader_not_found";
   }

   if (err == shader_codex_error::failed_to_open_file)
   {
      return "failed_to_open_file";
   }

   if (err == shader_codex_error::failed_to_insert_shader)
   {
      return "failed_to_insert_shader";
   }

   return "UNKNOWN";
}
auto to_err_code(shader_codex_error err) -> util::error_t
{
   return {{static_cast<int>(err), shader_codex_error_cat}};
}
