#include "tone/core/tokens.hpp"
#include "tone/core/lookup.hpp"

#include <fmt/format.h>

namespace tone::core {

    const lookup<std::string_view, reserved_token> operator_token_map{
            // Increment/Decrement
            {"++", reserved_token::inc},
            {"--", reserved_token::dec},

            // Basic arithmetic
            {"+", reserved_token::add},
            {"-", reserved_token::sub},
            {"*", reserved_token::mul},
            {"/", reserved_token::div},
            {"%", reserved_token::mod},

            // Bit operators
            {"~", reserved_token::bitwise_not},
            {"&", reserved_token::bitwise_and},
            {"|", reserved_token::bitwise_or},
            {"^", reserved_token::bitwise_xor},
            {"<<", reserved_token::shift_l},
            {">>", reserved_token::shift_r},

            // Assignment operators
            {"=", reserved_token::assign},

            // Logical operators
            {"!", reserved_token::logical_not},
            {"&&", reserved_token::logical_and},
            {"||", reserved_token::logical_or},

            // Comparison operators
            {"==", reserved_token::equal},
            {"!=", reserved_token::not_equal},
            {"<", reserved_token::less},
            {"<=", reserved_token::less_equal},
            {">", reserved_token::greater},
            {">=", reserved_token::greater_equal},

            // Misc.
            {",", reserved_token::comma},
            {";", reserved_token::semicolon},
            {":", reserved_token::colon},

            // Brackets
            {"(", reserved_token::open_paren},
            {")", reserved_token::close_paren},
            {"{", reserved_token::open_curly},
            {"}", reserved_token::close_curly},
            {"[", reserved_token::open_square},
            {"]", reserved_token::close_square},
    };


    const lookup<std::string_view, reserved_token> keyword_token_map{
            // Conditionals
            {"if", reserved_token::kw_if},
            {"else", reserved_token::kw_else},
            {"elif", reserved_token::kw_elif},

            // Loops
            {"for", reserved_token::kw_for},
            {"while", reserved_token::kw_while},

            // Defines
            {"var", reserved_token::kw_var},
            {"fn", reserved_token::kw_fn},

            // Control flow
            {"break", reserved_token::kw_break},
            {"continue", reserved_token::kw_continue},
            {"return", reserved_token::kw_return},

            // Types
            {"void", reserved_token::kw_void},
            {"real", reserved_token::kw_real},
            {"int", reserved_token::kw_int},
            {"bool", reserved_token::kw_bool},
            {"str", reserved_token::kw_str},

            // Constants
            {"true", reserved_token::kw_true},
            {"false", reserved_token::kw_false},
            {"null", reserved_token::kw_null},
    };

    const lookup<reserved_token, std::string_view> token_string_map = ([]() {
        std::vector<std::pair<reserved_token, std::string_view>> container;
        container.reserve(operator_token_map.size() + keyword_token_map.size());
        for (const auto& p : operator_token_map)
            container.emplace_back(p.second, p.first);
        for (const auto& p : keyword_token_map)
            container.emplace_back(p.second, p.first);
        return lookup<reserved_token, std::string_view>(std::move(container));
    })();

    class maximal_munch_compare
    {
    public:
        explicit maximal_munch_compare(std::size_t idx)
            : _idx(idx)
        {}
        bool operator()(char l, char r) const
        {
            return l < r;
        }
        bool operator()(std::pair<std::string_view, reserved_token> l, char r) const
        {
            return l.first.size() <= _idx || l.first[_idx] < r;
        }
        bool operator()(char l, std::pair<std::string_view, reserved_token> r) const
        {
            return r.first.size() > _idx && l < r.first[_idx];
        }
        bool operator()(std::pair<std::string_view, reserved_token> l,
                        std::pair<std::string_view, reserved_token> r) const
        {
            return r.first.size() > _idx &&
                   (l.first.size() < _idx || l.first[_idx] < r.first[_idx]);
        }

    private:
        std::size_t _idx;
    };

    std::optional<reserved_token> get_keyword(std::string_view word)
    {
        auto it = keyword_token_map.find(word);
        return it == keyword_token_map.end() ? std::nullopt : std::make_optional(it->second);
    }

    std::optional<reserved_token> get_operator(push_back_stream& stream)
    {
        auto candidates = std::make_pair(operator_token_map.begin(), operator_token_map.end());

        std::optional<reserved_token> tok;
        std::size_t match_size = 0;

        std::stack<character_t> chars;
        for (std::size_t idx = 0; candidates.first != candidates.second; ++idx)
        {
            chars.push(stream());

            candidates = std::equal_range(candidates.first, candidates.second, char(chars.top()),
                                          maximal_munch_compare(idx));

            if (candidates.first != candidates.second && candidates.first->first.size() == idx + 1)
            {
                match_size = idx + 1;
                tok = candidates.first->second;
            }
        }

        while (chars.size() > match_size)
        {
            stream.push_back(chars.top());
            chars.pop();
        }

        return tok;
    }
    std::string_view reserved_token_to_string(reserved_token tok)
    {
        auto it = token_string_map.find(tok);
        if (it != token_string_map.end())
            return it->second;
        else
            return "!!INVALID!!";
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// `token` class
    ////////////////////////////////////////////////////////////////////////////////////////////////

    token::token(token::value_type value, std::size_t line_number, std::size_t char_index)
        : _value(value)
        , _line_num(line_number)
        , _char_index(char_index)
    {}

    bool token::is_reserved_token() const
    {
        return std::holds_alternative<reserved_token>(_value);
    }
    bool token::is_identifier() const
    {
        return std::holds_alternative<identifier>(_value);
    }
    bool token::is_bool() const
    {
        return std::holds_alternative<bool>(_value);
    }
    bool token::is_real() const
    {
        return std::holds_alternative<double>(_value);
    }
    bool token::is_int() const
    {
        return std::holds_alternative<std::int64_t>(_value);
    }
    bool token::is_str() const
    {
        return std::holds_alternative<std::string>(_value);
    }
    bool token::is_null() const
    {
        return std::holds_alternative<null>(_value);
    }
    bool token::is_eof() const
    {
        return std::holds_alternative<eof>(_value);
    }


    reserved_token token::get_reserved_token() const
    {
        return std::get<reserved_token>(_value);
    }
    std::string_view token::get_identifier() const
    {
        return std::get<identifier>(_value).name;
    }
    bool token::get_bool() const
    {
        return std::get<bool>(_value);
    }
    double token::get_real() const
    {
        return std::get<double>(_value);
    }
    std::int64_t token::get_int() const
    {
        return std::get<std::int64_t>(_value);
    }
    std::string_view token::get_str() const
    {
        return std::get<std::string>(_value);
    }
    std::size_t token::get_line_number() const
    {
        return _line_num;
    }
    std::size_t token::get_char_index() const
    {
        return _char_index;
    }
    std::string token::dump() const
    {
        if (is_reserved_token())
            return fmt::format("Reserved: `{}`", reserved_token_to_string(get_reserved_token()));
        if (is_identifier())
            return fmt::format("Identifier: {}", get_identifier());
        if (is_bool())
            return fmt::format("Bool: {}", get_bool());
        if (is_real())
            return fmt::format("Real: {:.03f}", get_real());
        if (is_int())
            return fmt::format("Int: {}", get_int());
        if (is_str())
            return fmt::format("Str: \"{}\"", get_str());
        if (is_null())
            return "Null";
        if (is_eof())
            return "EOF";
        return "!!INVALID!!";
    }
}// namespace tone::core
