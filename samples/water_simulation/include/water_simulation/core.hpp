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
static constexpr float time_step = 0.016f;
static constexpr float water_radius = 1.0f;
static constexpr float kernel_radius = 3.0f * water_radius;
static constexpr float half_kernel_radius = kernel_radius / 2.0f;

static constexpr float rest_density = 25.0f; // higher means denser
static constexpr float viscosity_constant = 0.225f;
static constexpr float surface_tension_coefficient = 0.5f;
static constexpr float gravity_multiplier = 0.5f;

static constexpr float gravity = -9.81f;
static constexpr float scale_factor = 1.0f;
static constexpr float water_mass = 65.0f;

static constexpr float bound_damping = 0.5f;
static constexpr float edge = 15.0f;

static constexpr float poly6_constant = 315.0F / (65.0F * pi * my_pow(kernel_radius, 9.0F));
static constexpr float poly6_grad_constant = -945.f / (32.f * pi * my_pow(kernel_radius, 9.f));
static constexpr float spiky_constant = 15.0f / (pi * my_pow(kernel_radius, 6u));
static constexpr float spiky_grad_constant = -45.0f / (pi * my_pow(kernel_radius, 6u));

using image_index_t = util::strong_type<std::uint32_t, struct image_index_tag, util::arithmetic>;
