#pragma once

#include "tone/core/expression_tree.hpp"
#include "tone/core/type.hpp"
#include "tone/core/tokenizer.hpp"

namespace tone::core {
    class compile_context;

    node_ptr parse_expression_tree(compile_context& context, token_iterator& it, type_handle type_id, bool lvalue, bool allow_comma, bool allow_empty);
}