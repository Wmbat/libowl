/**
 * @file libowl/chrono.hpp
 * @author wmbat-dev@protonmail.com
 * @date
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBOWL_CHRONO_HPP_
#define LIBOWL_CHRONO_HPP_

#include <chrono>

namespace owl::inline v0
{
   template <typename Duration>
   using utc_time = std::chrono::utc_time<Duration>;
   using utc_second = utc_time<std::chrono::seconds>;
   using utc_millisecond = utc_time<std::chrono::milliseconds>;
   using utc_microsecond = utc_time<std::chrono::microseconds>;
   using utc_nanosecond = utc_time<std::chrono::nanoseconds>;

   template <typename Duration>
   using sys_time = std::chrono::sys_time<Duration>;
   using sys_second = sys_time<std::chrono::seconds>;
   using sys_millisecond = sys_time<std::chrono::milliseconds>;
   using sys_microsecond = sys_time<std::chrono::microseconds>;
   using sys_nanosecond = sys_time<std::chrono::nanoseconds>;
} // namespace owl::inline v0

#endif // LIBOWL_CHRONO_HPP_
