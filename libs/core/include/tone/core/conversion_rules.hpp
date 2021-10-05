#pragma once

#include "tone/core/type.hpp"

namespace tone::core {
    bool is_convertable(type_handle type_from, bool lvalue_from, type_handle type_to, bool lvalue_to);
}