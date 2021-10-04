#pragma once

#include "tone/core/push_back_stream.hpp"
#include "tone/core/tokens.hpp"

#include <iterator>

namespace tone::core {
    token tokenize(push_back_stream& stream);
} // namespace tone::core