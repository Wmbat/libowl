#ifndef LIBGERBIL_ASSERT_DETAIL_SOURCE_LOCATION_HPP_
#define LIBGERBIL_ASSERT_DETAIL_SOURCE_LOCATION_HPP_

#include <cstdint>

namespace gerbil::inline v0
{
   namespace detail
   {
      /**
       *
       */
      class source_location
      {
      public:
         constexpr source_location(char const* file = __builtin_FILE(), // NOLINT
                                   char const* function = __builtin_FUNCTION(),
                                   std::size_t line = __builtin_LINE()) :
            m_file_name(file),
            m_function_name(function), m_line(line)
         {}

         constexpr auto line() -> std::size_t { return m_line; }
         constexpr auto file_name() -> char const* { return m_file_name; }
         constexpr auto function_name() -> char const* { return m_function_name; }

      private:
         char const* const m_file_name = nullptr;
         char const* const m_function_name = nullptr;
         std::size_t const m_line = 0;
      };
   } // namespace detail
} // namespace gerbil::inline v0

#endif // LIBGERBIL_ASSERT_DETAIL_SOURCE_LOCATION_HPP_
