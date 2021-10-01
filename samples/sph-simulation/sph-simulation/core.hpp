#pragma once

#include <libmannele/logging/logger.hpp>
#include <libmannele/maths/maths.hpp>

#include <libreglisse/result.hpp>

#include <glm/ext/vector_uint3_sized.hpp>
#include <glm/vec3.hpp>

#include <spdlog/fmt/bundled/core.h>

#include <chrono>
#include <concepts>
#include <execution>
#include <filesystem>
#include <numbers>

#define IS_GCC (defined(__GNUC__) && !defined(__clang__))
#define IS_CLANG defined(__clang__)

using namespace std::literals::chrono_literals;

using filepath = std::filesystem::path;

inline static const auto asset_default_dir = filepath("../assets/"); // NOLINT

template <typename Any, typename Ratio = std::ratio<1>>
using duration = std::chrono::duration<Any, Ratio>;

template <std::ranges::input_range Range, typename Fun>
auto parallel_for(Range&& range, Fun&& fun)
{
   return std::for_each(std::execution::par, std::begin(range), std::end(range),
                        std::forward<Fun>(fun));
}

static constexpr std::uint32_t image_width = 1920;
static constexpr std::uint32_t image_height = 1080;

static constexpr float pi = std::numbers::pi_v<float>;
static constexpr float gravity = -9.81f;

static constexpr std::size_t max_frames = 30 * 20;
