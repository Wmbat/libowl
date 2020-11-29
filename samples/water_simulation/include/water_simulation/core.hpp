#pragma once

#include <water_simulation/maths.hpp>

#include <util/error.hpp>
#include <util/logger.hpp>

#include <monads/result.hpp>

#include <glm/ext/vector_uint3_sized.hpp>
#include <glm/vec3.hpp>

#include <spdlog/fmt/bundled/core.h>

#include <chrono>
#include <filesystem>
#include <numbers>

using namespace std::literals::chrono_literals;

template <typename Any>
using result = monad::result<Any, util::error_t>;

using filepath = std::filesystem::path;

template <typename Any, typename Ratio = std::ratio<1>>
using duration = std::chrono::duration<Any, Ratio>;

template <typename Any>
auto handle_err(Any&& result, util::logger_wrapper logger)
{
   if (auto err = result.error())
   {
      logger.error("{} error: {}", err->value().category().name(), err->value().message());

      std::exit(EXIT_FAILURE);
   }

   return std::forward<Any>(result).value().value();
}

static constexpr float pi = std::numbers::pi_v<float>;
static constexpr float gravity = -9.81f;
static constexpr float gravity_multiplier = 0.55f;

static constexpr float bound_damping = 0.5f;
static constexpr float edge = 15.0f;

using image_index_t = util::strong_type<std::uint32_t, struct image_index_tag, util::arithmetic>;

/*
static constexpr float default_rest_density = 25.0f; // higher means denser
static constexpr float default_viscosity_constant = 0.225f;
static constexpr float default_surface_tension_coefficient = 0.5f;
static constexpr float default_gravity_multiplier = 0.5f;

   float time_step = 0.016f;
   float water_radius = 1.0f;

   float scale_factor = 1.0f;
   float water_mass = 65.0f;
*/

struct settings
{
   duration<float, std::milli> time_step = 16ms;
   float water_radius = 1.0f;

   float scale_factor = 1.0f;
   float water_mass = 65.0f;

   float rest_density = 30.0f; // higher means denser
   float viscosity_constant = 0.5f;
   float surface_tension_coefficient = 0.25f;
   float gravity_multiplier = 0.56f;
   float kernel_multiplier = 3.0f;

   [[nodiscard]] inline auto kernel_radius() const -> float
   {
      return water_radius * kernel_multiplier;
   }
};

template <>
struct fmt::formatter<glm::vec3>
{
   char presentation = 'f';

   constexpr auto parse(format_parse_context& ctx)
   {
      auto it = ctx.begin(), end = ctx.end();
      if (it != end && (*it == 'f' || *it == 'e'))
         presentation = *it++;

      // Check if reached the end of the range:
      if (it != end && *it != '}')
         throw format_error("invalid format");

      // Return an iterator past the end of the parsed range:
      return it;
   }

   template <typename FormatContext>
   auto format(const glm::vec3& v, FormatContext& ctx)
   {
      return format_to(ctx.out(), presentation == 'f' ? "({:f}, {:f}, {:f})" : "({:e}, {:e}, {:e})",
                       v.x, v.y, v.z); // NOLINT
   }
};

template <>
struct fmt::formatter<glm::u64vec3>
{
   constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

   template <typename Context>
   auto format(const glm::u64vec3& p, Context& ctx)
   {
      return format_to(ctx.out(), "({}, {}, {})", p.x, p.y, p.z);
   }
};

namespace std
{
   template <>
   struct hash<glm::u64vec3>
   {
      auto operator()(const glm::u64vec3& v) const -> size_t
      {
         return ((hash<size_t>()(v.x) ^ (hash<size_t>()(v.y) << 1)) >> 1) ^
            (hash<std::size_t>()(v.z) << 1);
      }
   };
} // namespace std
