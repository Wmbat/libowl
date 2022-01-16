#ifndef LIBOWL_DETAIL_VISIT_HELPER_HPP_
#define LIBOWL_DETAIL_VISIT_HELPER_HPP_

namespace owl::inline v0
{
   namespace detail
   {
      template <typename... Types>
      struct overloaded : Types...
      {
         using Types::operator()...;
      };

      template <typename... Types>
      overloaded(Types...) -> overloaded<Types...>;
   } // namespace detail
} // namespace owl::inline v0

#endif // LIBOWL_DETAIL_VISIT_HELPER_HPP_
