#pragma once

#include <util/error.hpp>
#include <util/logger.hpp>

#include <monads/result.hpp>

#include <filesystem>

template <typename Any>
using result = monad::result<Any, util::error_t>;

using filepath = std::filesystem::path;

template <typename Any>
auto handle_err(Any&& result, const std::shared_ptr<util::logger>& p_logger)
{
   if (auto err = result.error())
   {
      util::log_error(p_logger, "error: {}", err->value().message());

      std::exit(EXIT_FAILURE);
   }

   return std::forward<Any>(result).value().value();
}

template <typename Any>
constexpr auto my_pow(Any num, unsigned int pow) -> Any
{
   // NOLINTNEXTLINE
   return (pow >= sizeof(unsigned int) * 8) ? 0 : pow == 0 ? 1 : num * my_pow(num, pow - 1);
}

using image_index_t = util::strong_type<std::uint32_t, struct image_index_tag, util::arithmetic>;
