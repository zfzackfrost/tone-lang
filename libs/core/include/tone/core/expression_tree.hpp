#pragma once

#include "tone/core/tokens.hpp"
#include "tone/core/type.hpp"
#include <variant>
#include <memory>
#include <vector>

namespace tone::core {
    enum class node_operation
    {
        param,

        pre_increment,
        pre_decrement,
        post_increment,
        post_decrement,

        unary_minus,
        unary_plus,


        add,
        sub,
        mul,
        div,
        mod,

        bitwise_not,
        bitwise_and,
        bitwise_or,
        bitwise_xor,
        shift_l,
        shift_r,

        assign,
        add_assign,
        sub_assign,
        mul_assign,
        div_assign,
        mod_assign,

        equal,
        not_equal,
        less,
        greater,
        less_equal,
        greater_equal,

        comma,
        index,

        logical_not,
        logical_and,
        logical_or,

        call,

    };

    struct node;
    using node_ptr = std::unique_ptr<node>;

    using node_value =
            std::variant<node_operation, std::u16string, std::int64_t, double, bool, identifier>;

    class compile_context;

    struct node
    {
    public:
        node(compile_context& context, node_value value, std::vector<node_ptr> children,
             std::size_t line_number, std::size_t char_index);

        [[nodiscard]] const node_value& get_value() const;
        [[nodiscard]] const std::vector<node_ptr>& get_children() const;
        [[nodiscard]] type_handle get_type_id() const;
        [[nodiscard]] bool is_lvalue() const;

        void check_conversion(type_handle type_id, bool lvalue);
        void check_any_conversion(std::initializer_list<type_handle> type_ids, bool lvalue);

        [[nodiscard]] bool is_node_operation() const;
        [[nodiscard]] bool is_identifier() const;
        [[nodiscard]] bool is_bool() const;
        [[nodiscard]] bool is_real() const;
        [[nodiscard]] bool is_int() const;
        [[nodiscard]] bool is_numeric() const;
        [[nodiscard]] bool is_str() const;
    private:
        node_value _value;
        std::vector<node_ptr> _children;
        type_handle _type_id;
        bool _lvalue : 1;
        std::size_t _line_number;
        std::size_t _char_index;
    };
} // namespace tone::core