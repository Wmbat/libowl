#ifndef LIBMANNELE_CONCEPTS_HPP
#define LIBMANNELE_CONCEPTS_HPP

#include <concepts>

namespace mannele
{
   template <typename Type>
   concept number = std::integral<Type> || std::floating_point<Type>;
} // namespace mannele

#endif // LIBMANNELE_CONCEPTS_HPP
