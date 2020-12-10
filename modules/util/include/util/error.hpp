#pragma once

#include <util/strong_type.hpp>

#include <monads/result.hpp>

#include <system_error>

namespace vml
{
   using error_t = util::strong_type<std::error_condition, struct error_condition_tag>;

   class runtime_error : public std::runtime_error
   {
   public:
      runtime_error(error_t e);
      runtime_error(error_t e, const std::string& what_arg);
      runtime_error(error_t e, const std::string_view what_arg);
      runtime_error(int value, const std::error_category& category);
      runtime_error(int value, const std::error_category& category, const std::string& what_arg);
      runtime_error(int value, const std::error_category& category,
                    const std::string_view what_arg);

      [[nodiscard]] auto condition() const noexcept -> const error_t&;

      [[nodiscard]] auto what() const noexcept -> const char* override;

   private:
      error_t m_error_condition;
   };
} // namespace vml

namespace util
{

   using error_t = strong_type<std::error_code, struct error_code_tag>;

   template <typename Any>
   using result = monad::result<Any, error_t>;
} // namespace util
