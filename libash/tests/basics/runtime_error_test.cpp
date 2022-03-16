#include <libash/runtime_error.hpp>

#include "fmt/core.h"

#include <gtest/gtest.h>
#include <magic_enum.hpp>

enum struct error_code
{
   e_one,
   e_two
};

struct testing_error_category : std::error_category
{
   [[nodiscard]] auto name() const noexcept -> const char* override { return "ash::instance"; }
   [[nodiscard]] auto message(int err) const -> std::string override
   {
      return std::string(magic_enum::enum_name(static_cast<error_code>(err)));
   }
};

TEST(runtime_error, basic) // NOLINT
{
   auto category = testing_error_category();
   auto error =
      ash::runtime_error(std::error_condition({static_cast<int>(error_code::e_one), category}), {});

   fmt::print("{}\n", error.what());

   ASSERT_EQ(error.condition().category(), category);
   ASSERT_EQ(error.condition().value(), static_cast<int>(error_code::e_one));
   ASSERT_EQ(error.condition().message(), magic_enum::enum_name(error_code::e_one));
}
