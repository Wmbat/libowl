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
         constexpr source_location(const char* file = __builtin_FILE(), // NOLINT
                                   const char* function = __builtin_FUNCTION(),
                                   std::size_t line = __builtin_LINE()) :
            m_file_name(file),
            m_function_name(function), m_line(line)
         {}

         constexpr auto line() -> std::size_t { return m_line; }
         constexpr auto file_name() -> const char* { return m_file_name; }
         constexpr auto function_name() -> const char* { return m_function_name; }

      private:
         const char* const m_file_name = nullptr;
         const char* const m_function_name = nullptr;
         const std::size_t m_line = 0;
      };
   } // namespace detail
} // namespace gerbil::inline v0

#endif // LIBGERBIL_ASSERT_DETAIL_SOURCE_LOCATION_HPP_
