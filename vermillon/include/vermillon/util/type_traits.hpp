/**
 * @file type_traits.hpp.
 * @author wmbat wmbat@protonmail.com.
 * @date 20th of July, 2020.
 * @copyright MIT License.
 *
 */

#pragma once

#include <type_traits>

namespace util
{
   struct nonesuch
   {
      nonesuch(nonesuch const &) = delete;
      nonesuch(nonesuch &&) noexcept = delete;
      ~nonesuch() = delete;

      void operator=(nonesuch const &) = delete;
      void operator=(nonesuch &&) = delete;
   };

   namespace detail
   {
      template <class Default, class AlwaysVoid, template <class...> class Op, class... Args>
      struct detector
      {
         using value_t = std::false_type;
         using type = Default;
      };

      template <class Default, template <class...> class Op, class... Args>
      struct detector<Default, std::void_t<Op<Args...>>, Op, Args...>
      {
         using value_t = std::true_type;
         using type = Op<Args...>;
      };

   } // namespace detail

   template <template <class...> class Op, class... Args>
   using is_detected = typename detail::detector<nonesuch, void, Op, Args...>::value_t;

   template <template <class...> class Op, class... Args>
   using detected_t = typename detail::detector<nonesuch, void, Op, Args...>::type;

   template <class Default, template <class...> class Op, class... Args>
   using detected_or = detail::detector<Default, void, Op, Args...>;

} // namespace util
