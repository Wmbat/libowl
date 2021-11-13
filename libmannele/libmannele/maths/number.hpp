/**
 * @file libmannele/concepts.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date Monday, 22nd of September 2021
 * @brief 
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBMANNELE_CONCEPTS_HPP_
#define LIBMANNELE_CONCEPTS_HPP_

#include <concepts>

namespace mannele
{
   template <typename Type>
   concept number = std::integral<Type> || std::floating_point<Type>;
} // namespace mannele

#endif // LIBMANNELE_CONCEPTS_HPP_
