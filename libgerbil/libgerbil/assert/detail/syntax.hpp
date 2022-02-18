#ifndef LIBGERBIL_ASSERT_DETAIL_SYNTAX_HPP_
#define LIBGERBIL_ASSERT_DETAIL_SYNTAX_HPP_

#include <fmt/color.h>

#include <optional>
#include <vector>

namespace gerbil::inline v0
{
   namespace detail
   {
      struct text_group
      {
         std::optional<fmt::color> colour;
         std::string text;
      };

      auto extract_text_groups(std::string const& expr) -> std::vector<text_group>;
   } // namespace detail
} // namespace gerbil::inline v0

#endif // LIBGERBIL_ASSERT_DETAIL_SYNTAX_HPP_
