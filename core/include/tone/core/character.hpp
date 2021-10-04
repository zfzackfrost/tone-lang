#pragma once

#include <functional>
#include <cstdint>

namespace tone::core {
    using character_t = int;
    using character_source_t = std::function<character_t()>;
}