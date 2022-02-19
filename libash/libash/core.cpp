#include <libash/core.hpp>

#include <libash/detail/vulkan.hpp>

namespace ash::inline v0
{
   auto detail::to_vulkan_version(mannele::semantic_version version) -> u32
   {
      return VK_MAKE_API_VERSION(0, version.major, version.minor, version.patch);
   }

   auto detail::from_vulkan_version(u32 version) -> mannele::semantic_version
   {
      return {.major = VK_VERSION_MAJOR(version),
              .minor = VK_VERSION_MINOR(version),
              .patch = VK_VERSION_PATCH(version)};
   }
} // namespace ash::inline v0
