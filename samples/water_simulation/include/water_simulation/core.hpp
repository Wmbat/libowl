#pragma once

#include <water_simulation/maths.hpp>

#include <vermillon/util/error.hpp>
#include <vermillon/util/logger.hpp>

#include <monads/result.hpp>

#include <glm/ext/vector_uint3_sized.hpp>
#include <glm/vec3.hpp>

#include <spdlog/fmt/bundled/core.h>

#include <chrono>
#include <execution>
#include <filesystem>
#include <numbers>
#include <ranges>

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

template <std::ranges::input_range Range, typename Fun>
auto parallel_for(Range&& range, Fun&& fun)
{
   return std::for_each(std::execution::par_unseq, std::begin(range), std::end(range),
                        std::forward<Fun>(fun));
}

static constexpr std::uint32_t image_width = 1920;
static constexpr std::uint32_t image_height = 1080;

static constexpr float pi = std::numbers::pi_v<float>;
static constexpr float gravity = -9.81f;

static constexpr std::size_t max_frames = 30 * 20;

using image_index_t = util::strong_type<std::uint32_t, struct image_index_tag, util::arithmetic>;

struct settings
{
   duration<float, std::milli> time_step = 1ms;

   float gas_constant = 2000.0f;
   float rest_density = 1000.0f;
   float viscosity_constant = 300.0f;
   float surface_tension_coefficient = 0.5f;
   float gravity_multiplier = 1.0f;
   float kernel_multiplier = 5.0f;
   float scale_factor = 0.25f;

   float water_radius = 0.20f;
   float water_mass = rest_density * cube(water_radius * 2);

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
