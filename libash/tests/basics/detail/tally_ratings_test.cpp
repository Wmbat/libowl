#include <libash/detail/tally_ratings.hpp>

#include <gtest/gtest.h>

TEST(tally_ratings, basic) // NOLINT
{
   EXPECT_EQ(ash::detail::tally_ratings(1, 2, 3), 6);
   EXPECT_EQ(ash::detail::tally_ratings(1, 2, -1), -1);
}

TEST(tally_ratings, overflow) // NOLINT
{
   constexpr int max = std::numeric_limits<int>::max();
   constexpr int min = std::numeric_limits<int>::min();
   EXPECT_EQ(ash::detail::tally_ratings(max, 1), min);
   EXPECT_EQ(ash::detail::tally_ratings(max, max, -1), -1);

   EXPECT_EQ(ash::detail::tally_ratings(min, min, -1), -1);
   EXPECT_EQ(ash::detail::tally_ratings(min, min), 0);
}
