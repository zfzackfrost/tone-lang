#pragma once

#include "tone/core/push_back_stream.hpp"
#include "tone/core/tokens.hpp"

namespace tone::core {
    token tokenize(push_back_stream& stream);
}