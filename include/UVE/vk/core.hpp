#include <UVE/vk/vma/vk_mem_alloc.h>

#include <volk.h>

namespace vk
{
#if defined( NDEBUG )
   static constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
   static constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif
} // namespace vk
