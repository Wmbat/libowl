#include <libash/detail/tally_ratings.hpp>

#include <gtest/gtest.h>

TEST(tally_ratings, basic) // NOLINT
{
   EXPECT_EQ(ash::detail::tally_ratings(1, 2, 3), 6); 
   EXPECT_EQ(ash::detail::tally_ratings(1, 2, -1), -1); 
}
