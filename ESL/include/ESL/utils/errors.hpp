/**
 * MIT License
 *
 * Copyright (c) 2020 Wmbat
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <cstdint>
#include <optional>
#include <system_error>
#include <tuple>

namespace ESL
{
   template <typename type_>
   using ret_err = std::tuple<std::optional<type_>, std::error_condition>;

   enum class basic_error : std::uint32_t
   {
      e_none = 0,
      e_bad_alloc,
      e_out_of_bounds
   };

   class basic_category : public std::error_category
   {
   public:
      const char* name( ) const noexcept override { return "basic_category"; }
      std::string message( int value ) const override
      {
         switch ( static_cast<basic_error>( value ) )
         {
            case basic_error::e_none:
               return "Not an error";
               break;
            case basic_error::e_bad_alloc:
               return "bad allocation";
            default:
               return "unknown error";
         }
      }
   };

   std::error_category const& get_basic_category( )
   {
      static basic_category my_category;
      return my_category;
   }

   std::error_condition make_error_condition( basic_error e )
   {
      return std::error_condition( static_cast<int>( e ), get_basic_category( ) );
   }
} // namespace ESL

namespace std
{
   template <>
   struct is_error_condition_enum<ESL::basic_error> : std::true_type
   {
   };
} // namespace std
