#pragma once

#if !defined(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC)
#   define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif

#include <libcacao/util/error.hpp>
#include <libcacao/util/strong_type.hpp>

#include <libreglisse/result.hpp>
#include <libreglisse/try.hpp>

#include <vulkan/vulkan.hpp>

#include <span>
#include <system_error>

namespace cacao
{
   template <typename Any>
   auto to_array_proxy(std::span<Any> s) -> vk::ArrayProxyNoTemporaries<Any>
   {
      return vk::ArrayProxyNoTemporaries<Any>{s.size(), s.data()};
   }
} // namespace cacao

namespace vkn
{
   static constexpr cacao::count32_t expected_image_count = 3U;

   namespace detail
   {
#if defined(NDEBUG) && defined(CACAO_DEBUG_LOGGING)
      static constexpr bool ENABLE_VALIDATION_LAYERS = true;
#elif defined(NDEBUG)
      static constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
      static constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif
   } // namespace detail

   template <typename... args_>
   auto try_wrap(std::invocable<args_...> auto&& fun, args_&&... args)
      -> reglisse::result<std::invoke_result_t<decltype(fun), args_...>, vk::SystemError>
   {
      return reglisse::try_wrap<vk::SystemError>(std::forward<decltype(fun)>(fun),
                                              std::forward<args_>(args)...);
   }

   template <typename any_>
   class owning_handle
   {
   public:
      using value_type = any_;
      using pointer = any_*;
      using const_pointer = const any_*;

      /**
       * Allow direct access to the underlying handle functions
       */
      auto operator->() noexcept -> pointer { return &m_value.get(); }
      /**
       * Allow direct access to the underlying handle functions
       */
      auto operator->() const noexcept -> const_pointer { return &m_value.get(); }

      /**
       * Get the underlying handle
       */
      auto operator*() const noexcept -> value_type { return m_value.get(); }
      operator bool() const noexcept { return m_value.get(); } // NOLINT

      /**
       * Get the underlying handle
       */
      [[nodiscard]] auto value() const noexcept -> value_type { return m_value.get(); }

   protected:
      vk::UniqueHandle<any_, vk::DispatchLoaderDynamic> m_value; // NOLINT
   };

   // clang-format off
   template <typename any_>
   concept handle = requires(any_ a)
   {
      typename any_::value_type;

      { a.value() } -> std::same_as<typename any_::value_type>;
   };
   // clang-format on

   /**
    * A utility function to extract the undelying value under a vkn handle
    */
   constexpr auto value(const handle auto& h) { return h.value(); }
}; // namespace vkn
