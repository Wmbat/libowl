#pragma once

#include <util/strong_type.hpp>

namespace ecs
{
   using entity = util::strong_type<std::uint32_t, struct entity_tag>;
} // namespace ecs
