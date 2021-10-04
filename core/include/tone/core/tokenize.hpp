#pragma once

#include "tone/core/tokens.hpp"
#include "tone/core/push_back_stream.hpp"

namespace tone::core {
    token tokenize(push_back_stream& stream);
}