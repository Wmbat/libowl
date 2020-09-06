#pragma once

#if !defined(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC)
#   define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif

#include <util/logger.hpp>
#include <util/strong_type.hpp>

#include <monads/either.hpp>
#include <monads/result.hpp>

#include <vulkan/vulkan.hpp>

#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include <glslang/Public/ShaderLang.h>

#include <system_error>

namespace vkn
{
   static constexpr util::count32_t expected_image_count = 3u;

   namespace detail
   {
#if defined(NDEBUG)
      static constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
      static constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif
   } // namespace detail

   /**
    * The default error type within the vkn namespace.
    */
   struct error final
   {
      std::error_code type;
      vk::Result result{};
   };

   /**
    * An alias for an either monad using an error as the left type.
    */
   template <typename any_>
   using result = monad::result<any_, vkn::error>;

   /**
    * Class used for the dynamic loading of the Vulkan API functions.
    */
   class loader final
   {
   public:
      loader(const std::shared_ptr<util::logger>& p_logger = nullptr);

      /**
       * Load all vulkan functions based on the Vulkan instance.
       */
      void load_instance(const vk::Instance& instance) const;
      /**
       * Load all vulkan functions based on the Vulkan device.
       */
      void load_device(const vk::Device& device) const;

   private:
      std::shared_ptr<util::logger> mp_logger;

      vk::DynamicLoader m_dynamic_loader;

      inline static bool IS_GLSLANG_INIT = false;
   };

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
      operator bool() const noexcept { return m_value.get(); }

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
