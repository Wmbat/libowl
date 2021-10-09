#pragma once

#include <sph-simulation/core.hpp>
#include <sph-simulation/core/pipeline.hpp>

#include <libreglisse/operations/transform_err.hpp>
#include <libreglisse/try.hpp>

#include <libmannele/core.hpp>

enum struct pipeline_registry_error
{
   failed_to_insert_pipeline,
   pipeline_not_found
};

auto make_error_condition(pipeline_registry_error err) -> std::error_condition;

class pipeline_registry
{
   using graphics_map = std::unordered_map<std::size_t, pipeline<pipeline_type::graphics>>;
   using compute_map = std::unordered_map<std::size_t, pipeline<pipeline_type::compute>>;

public:
   using key_type = mannele::u64;

   template <pipeline_type Type>
   class lookup_v
   {
   public:
      lookup_v(pipeline<Type>* p_value) : mp_value(p_value) {}

      [[nodiscard]] auto value() const -> pipeline<Type>& { return *mp_value; }

   private:
      pipeline<Type>* mp_value;
   };

   template <pipeline_type Type>
   class insert_kv
   {
   public:
      insert_kv(key_type key, pipeline<Type>* p_value) : m_key(key), mp_value(p_value) {}

      [[nodiscard]] auto key() const -> const key_type& { return m_key; }
      [[nodiscard]] auto value() const -> pipeline<Type>& { return *mp_value; }

   private:
      key_type m_key;
      pipeline<Type>* mp_value;
   };

   template <pipeline_type Type>
   class remove_v
   {
   public:
      remove_v(pipeline<Type>&& value) : m_value(std::move(value)) {}

      [[nodiscard]] auto value() -> pipeline<Type>& { return m_value; }

      auto take() -> pipeline<Type> { return std::move(m_value); }

   private:
      pipeline<Type> m_value;
   };

public:
   pipeline_registry(mannele::log_ptr logger);

   auto insert(graphics_pipeline_create_info&& info)
      -> reglisse::result<insert_kv<pipeline_type::graphics>, pipeline_registry_error>;
   auto insert(compute_pipeline_create_info&& info)
      -> reglisse::result<insert_kv<pipeline_type::compute>, pipeline_registry_error>;

   template <pipeline_type Type>
   auto lookup(const key_type& key) -> reglisse::result<lookup_v<Type>, pipeline_registry_error>
   {
      using reglisse::transform_err;
      using reglisse::try_wrap;

      if constexpr (Type == pipeline_type::graphics)
      {
         return try_wrap<std::exception>([&] {
                   return lookup_v{&m_graphics_pipelines.at(key)};
                }) |
            transform_err([](const std::exception& /*err*/) {
                   return pipeline_registry_error::pipeline_not_found;
                });
      }
      else
      {
         return try_wrap<std::exception>([&] {
                   return lookup_v{&m_compute_pipelines.at(key)};
                }) |
            transform_err([](const std::exception& /*err*/) {
                   return pipeline_registry_error::pipeline_not_found;
                });
      }
   }

   template <pipeline_type Type>
   auto remove(const key_type& key) -> reglisse::result<remove_v<Type>, pipeline_registry_error>
   {
      using reglisse::err;
      using reglisse::ok;

      if constexpr (Type == pipeline_type::graphics)
      {
         auto it = m_graphics_pipelines.find(key);
         if (it != std::end(m_graphics_pipelines))
         {
            remove_v res{std::move(it->second)};

            m_graphics_pipelines.erase(key);

            return ok(std::move(res));
         }
      }
      else
      {
         auto it = m_compute_pipelines.find(key);
         if (it != std::end(m_compute_pipelines))
         {
            remove_v res{std::move(it->second)};

            m_graphics_pipelines.erase(key);

            return ok(std::move(res));
         }
      }

      return err(pipeline_registry_error::pipeline_not_found);
   }

private:
   graphics_map m_graphics_pipelines;
   compute_map m_compute_pipelines;

   mannele::log_ptr m_logger;

   mannele::u64 id_counter{0};
};
