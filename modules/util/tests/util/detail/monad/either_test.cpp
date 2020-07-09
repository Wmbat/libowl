#include <util/monad/either.hpp>

#include <gtest/gtest.h>

#if !defined(SUITE_NAME)
#   define SUITE_NAME either_monad
#endif

TEST(SUITE_NAME, left_copy_ctor)
{
   {
      const auto left = util::monad::to_left(10);
      util::either<int, std::string> int_either{left};

      EXPECT_EQ(int_either.is_left(), true);
      EXPECT_EQ(int_either.left().value(), 10);
   }

   {
      const auto left = util::monad::to_left(std::string{"test string"});
      util::either<std::string, std::string> data{left};

      EXPECT_EQ(data.is_left(), true);
      EXPECT_EQ(data.left().value(), std::string{"test string"});
   }
}
