/*
 *  Copyright (C) 2018-2019 Wmbat
 *
 *  wmbat@protonmail.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  You should have received a copy of the GNU General Public License
 *  GNU General Public License for more details.
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LUCIOLE_UTILITIES_ENUM_OPERATORS
#define LUCIOLE_UTILITIES_ENUM_OPERATORS

#include <type_traits>

/**
 * @brief Struct used to do a compile-time check on whether an
 * enum can use the overleaded binary operators.
 *
 * @tparam T The Enum Class for whom the overleaded operators will
 * be enabled.
 */
template <typename T>
struct enable_bitmask_operators
{
   static_assert( std::is_enum_v<T>, "Template parameter is not of enum type." );

   static constexpr bool enable = false;
};

/**
 * @brief Macro to facilitate the enabling of the binary operators.
 */
#define ENABLE_BITMASK_OPERATORS( enum_class )                                                                                             \
   template <>                                                                                                                             \
   struct enable_bitmask_operators<enum_class>                                                                                             \
   {                                                                                                                                       \
      static_assert( std::is_enum_v<enum_class>, "Template parameter is not of enum type." );                                              \
                                                                                                                                           \
      static constexpr bool enable = true;                                                                                                 \
   };

/**
 * @brief Overload the bitwise or operator.
 *
 * @tparam T The Enum Class to compare
 * @param lhs The Enum value on the left hand side of the operator.
 * @param rhs The Enum value on the right hand side of the operator.
 * @return std::enable_if_t<enable_bitmask_operators<T>::enable, T> Returns a new instance of T.
 */
template <typename T>
typename std::enable_if_t<enable_bitmask_operators<T>::enable, T> operator|( T lhs, T rhs )
{
   static_assert( std::is_enum_v<T>, "Template parameter is not of enum type." );

   return static_cast<T>( static_cast<std::underlying_type_t<T>>( lhs ) | static_cast<std::underlying_type_t<T>>( rhs ) );
}

template <typename T>
typename std::enable_if_t<enable_bitmask_operators<T>::enable, T> operator&( T lhs, T rhs )
{
   static_assert( std::is_enum_v<T>, "Template parameter is not of enum type." );

   return static_cast<T>( static_cast<std::underlying_type_t<T>>( lhs ) & static_cast<std::underlying_type_t<T>>( rhs ) );
}

template <typename T>
typename std::enable_if_t<enable_bitmask_operators<T>::enable, T> operator^( T lhs, T rhs )
{
   static_assert( std::is_enum_v<T>, "Template parameter is not of enum type." );

   return static_cast<T>( static_cast<std::underlying_type_t<T>>( lhs ) | static_cast<std::underlying_type_t<T>>( rhs ) );
}

template <typename T>
typename std::enable_if_t<enable_bitmask_operators<T>::enable, T> operator~( T rhs )
{
   static_assert( std::is_enum_v<T>, "Template parameter is not of enum type." );

   return static_cast<T>( ~static_cast<std::underlying_type_t<T>>( rhs ) );
}

template <typename T>
typename std::enable_if_t<enable_bitmask_operators<T>::enable, T> operator|=( T& lhs, T rhs )
{
   static_assert( std::is_enum_v<T>, "Template parameter is not of enum type." );

   lhs = static_cast<T>( static_cast<std::underlying_type_t<T>>( lhs ) | static_cast<std::underlying_type_t<T>>( rhs ) );

   return lhs;
}

template <typename T>
typename std::enable_if_t<enable_bitmask_operators<T>::enable, T> operator&=( T& lhs, T rhs )
{
   static_assert( std::is_enum_v<T>, "Template parameter is not of enum type." );

   lhs = static_cast<T>( static_cast<std::underlying_type_t<T>>( lhs ) & static_cast<std::underlying_type_t<T>>( rhs ) );

   return lhs;
}

template <typename T>
typename std::enable_if_t<enable_bitmask_operators<T>::enable, T> operator^=( T& lhs, T rhs )
{
   static_assert( std::is_enum_v<T>, "Template parameter is not of enum type." );

   lhs = static_cast<T>( static_cast<std::underlying_type_t<T>>( lhs ) ^ static_cast<std::underlying_type_t<T>>( rhs ) );

   return lhs;
}

#endif // LUCIOLE_UTILITIES_ENUM_OPERATORS
