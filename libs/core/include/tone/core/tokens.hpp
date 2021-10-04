#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "tone/core/push_back_stream.hpp"

namespace tone::core {
    enum class reserved_token : std::uint16_t
    {
        // Increment, Decrement operators
        inc,
        dec,

        // Basic arithmetic operators
        add,
        sub,
        mul,
        div,
        mod,

        // Bitwise operators
        bitwise_not,
        bitwise_and,
        bitwise_or,
        bitwise_xor,
        shift_l,
        shift_r,

        // Assignment operators
        assign,
        add_assign,
        sub_assign,
        mul_assign,
        div_assign,
        mod_assign,

        // Logical operators
        logical_not,
        logical_and,
        logical_or,

        // Comparison operators
        equal,
        not_equal,
        less,
        greater,
        less_equal,
        greater_equal,

        // Misc. operators
        comma,
        semicolon,
        colon,
        scope,

        // Brackets, Braces
        open_paren,
        close_paren,
        open_curly,
        close_curly,
        open_square,
        close_square,

        // Keywords - Conditions
        kw_if,
        kw_else,
        kw_elif,

        // Keywords - Loops
        kw_for,
        kw_while,

        // Keywords - Definitions
        kw_var,
        kw_fn,

        // Keywords - Flow control
        kw_break,
        kw_continue,
        kw_return,

        // Keywords -Types
        kw_void,
        kw_real,
        kw_int,
        kw_bool,
        kw_str,

        // Keywords - Constants
        kw_constant_true,
        kw_constant_false,
        kw_constant_null,
    };

    std::optional<reserved_token> get_keyword(std::string_view word);
    std::optional<reserved_token> get_operator(push_back_stream& stream);

    struct identifier final {
        std::string name;
    };

    struct eof final {
    };

    struct null final {
    };

    class token
    {
    public:
        using value_type = std::variant<reserved_token, identifier, bool, double, std::int64_t,
                                        std::u16string, null, eof>;

        token(value_type value, std::size_t line_number, std::size_t char_index);
        token();

        bool operator==(const token& rhs) const;

        [[nodiscard]] bool is_reserved_token() const;
        [[nodiscard]] bool is_identifier() const;
        [[nodiscard]] bool is_bool() const;
        [[nodiscard]] bool is_real() const;
        [[nodiscard]] bool is_int() const;
        [[nodiscard]] bool is_str() const;
        [[nodiscard]] bool is_null() const;
        [[nodiscard]] bool is_eof() const;

        [[nodiscard]] reserved_token get_reserved_token() const;
        [[nodiscard]] std::string_view get_identifier() const;
        [[nodiscard]] bool get_bool() const;
        [[nodiscard]] double get_real() const;
        [[nodiscard]] std::int64_t get_int() const;
        [[nodiscard]] std::u16string get_str() const;

        [[nodiscard]] std::size_t get_line_number() const;
        [[nodiscard]] std::size_t get_char_index() const;

        std::string dump() const;

    private:
        value_type _value;
        std::size_t _line_num;
        std::size_t _char_index;
    };

} // namespace tone::core