#pragma once

#include "epona_core/vk/core.hpp"

#include <cassert>
#include <system_error>

namespace core::vk::details
{
   struct error
   {
      std::error_code type;
      VkResult result = VK_SUCCESS;
   };

   template <typename type_>
   class result
   {
   public:
      using value_type = type_;

   public:
      result(const value_type& value) : val{value}, is_init{true} {}
      result(value_type&& value) : val{std::move(value)}, is_init{true} {}
      result(error err) : err{err}, is_init{false} {}
      result(std::error_code err_code, VkResult result = VK_SUCCESS) :
         err{.type = err_code, .result = result}, is_init{false}
      {}
      ~result() { destroy(); }

      const value_type* operator->() const
      {
         assert(is_init);

         return &val;
      }
      value_type* operator->()
      {
         assert(is_init);

         return &val;
      }

      const value_type& operator*() const&
      {
         assert(is_init);

         return val;
      }
      value_type& operator*() &
      {
         assert(is_init);

         return val;
      }
      value_type&& operator*() &&
      {
         assert(is_init);

         return std::move(val);
      }

      const value_type& value() const&
      {
         assert(is_init);

         return val;
      }
      value_type& value() &
      {
         assert(is_init);

         return val;
      }

      const value_type&& value() const&&
      {
         assert(is_init);

         return std::move(val);
      }
      value_type&& value() &&
      {
         assert(is_init);

         return std::move(val);
      }

      [[nodiscard]] std::error_code error_type() const
      {
         assert(!is_init);

         return err.type;
      }
      [[nodiscard]] VkResult error_result() const
      {
         assert(!is_init);

         return err.result;
      }

      [[nodiscard]] bool has_value() const { return is_init; }
      explicit operator bool() const { return is_init; }

   private:
      void destroy()
      {
         if (is_init)
         {
            val.~value_type();
         }
      }

      union
      {
         value_type val;
         struct error err;
      };

      bool is_init;
   };
} // namespace core::vk::details
