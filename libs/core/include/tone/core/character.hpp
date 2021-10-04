#pragma once

#include <cstdint>
#include <functional>

namespace tone::core {
    using character_t = int;
    using character_source_t = std::function<character_t()>;
}// namespace tone::core