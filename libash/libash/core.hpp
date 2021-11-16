#ifndef LIBASH_CORE_HPP_
#define LIBASH_CORE_HPP_

#include <libmannele/core.hpp>
#include <libmannele/core/semantic_version.hpp>

namespace ash
{
   using mannele::f32;
   using mannele::f64;
   using mannele::i16;
   using mannele::i32;
   using mannele::i64;
   using mannele::i8;
   using mannele::u16;
   using mannele::u32;
   using mannele::u64;
   using mannele::u8;

#if defined(NDEBUG)
   static constexpr bool enable_validation_layers = false;
#else
   static constexpr bool enable_validation_layers = true;
#endif

   namespace detail
   {
      /**
       * @brief Converts a mannele::semantic_version into a single u32 value
       * used by the vulkan API.
       *
       * @param [in] version The semantic version to convert into a single unsigned integer
       *
       * @return A 32 bit unsigned integer representation of the semantic version.
       */
      auto to_vulkan_version(mannele::semantic_version version) -> u32;

      auto from_vulkan_version(u32 version) -> mannele::semantic_version;
   } // namespace detail
} // namespace ash

#endif // LIBASH_CORE_HPP_
