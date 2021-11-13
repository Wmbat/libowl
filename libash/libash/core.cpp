#include <libash/core.hpp>

#include <libash/detail/vulkan.hpp>

namespace ash
{
   auto detail::to_vulkan_version(mannele::semantic_version version) -> u32
   {
      return VK_MAKE_API_VERSION(0, version.major, version.minor, version.patch);
   }
} // namespace ash
