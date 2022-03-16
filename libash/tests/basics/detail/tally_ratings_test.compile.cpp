#include <libash/detail/tally_ratings.hpp>

#include <limits>

static_assert(ash::detail::tally_ratings(1, 2, 3) == 6);   // NOLINT
static_assert(ash::detail::tally_ratings(-1, 2, 3) == -1); // NOLINT

constexpr int max = std::numeric_limits<int>::max();
constexpr int min = std::numeric_limits<int>::min();

static_assert(ash::detail::tally_ratings(max, max, -1) == -1);
static_assert(ash::detail::tally_ratings(min, min, -1) == -1);
