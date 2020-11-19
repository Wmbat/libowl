#pragma once

#include <util/error.hpp>
#include <util/logger.hpp>

#include <monads/result.hpp>

#include <filesystem>
#include <numbers>

template <typename Any>
using result = monad::result<Any, util::error_t>;

using filepath = std::filesystem::path;

template <typename Any>
auto handle_err(Any&& result, const std::shared_ptr<util::logger>& p_logger)
{
   if (auto err = result.error())
   {
      util::log_error(p_logger, "{} error: {}", err->value().category().name(),
                      err->value().message());

      std::exit(EXIT_FAILURE);
   }

   return std::forward<Any>(result).value().value();
}

// clang-format off
template <typename Any>
concept number = std::integral<Any> || std::floating_point<Any>;
// clang-format on

template <number Any>
constexpr auto my_pow(Any num, unsigned int pow) -> Any
{
   // NOLINTNEXTLINE
   return (pow >= sizeof(unsigned int) * 8) ? 0 : pow == 0 ? 1 : num * my_pow(num, pow - 1);
}

template <number Any>
constexpr auto square(Any num) -> Any
{
   return num * num;
}

template <number Any>
constexpr auto cube(Any num) -> Any
{
   return num * num * num;
};

static constexpr float pi = std::numbers::pi_v<float>;
static constexpr float gravity = -9.81f;

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
   float time_step = 0.016f;
   float water_radius = 1.0f;

   float scale_factor = 1.0f;
   float water_mass = 65.0f;

   float rest_density = 25.0f; // higher means denser
   float viscosity_constant = 0.225f;
   float surface_tension_coefficient = 0.5f;
   float gravity_multiplier = 0.5f;

   [[nodiscard]] inline auto kernel_radius() const -> float { return water_radius * 3.0f; }
};
