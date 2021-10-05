#include "tone/core/errors.hpp"

#include <cstdlib>
#include <fmt/format.h>

#include <utility>

namespace tone::core {

    error::error(std::string message, std::size_t line_number, std::size_t char_index)
        : _message(std::move(message))
        , _line_number(line_number)
        , _char_index(char_index)
    {}
    const char* error::what() const noexcept
    {
        return _message.c_str();
    }
    std::size_t error::line_number() const
    {
        return _line_number;
    }
    std::size_t error::char_index() const
    {
        return _char_index;
    }


    error parsing_error(const char* message, std::size_t line_number, std::size_t char_index)
    {
        std::string error_message("Parsing error: ");
        error_message += message;
        return {std::move(error_message), line_number, char_index};
    }
    error syntax_error(std::string_view message, size_t line_number, size_t char_index)
    {
        std::string msg("Syntax error: ");
        msg += message;
        return {std::move(msg), line_number, char_index};
    }
    error unexpected_error(std::string_view unexpected, std::size_t line_number,
                           std::size_t char_index)
    {
        std::string message("Unexpected '");
        message += unexpected;
        message += "'";
        return {std::move(message), line_number, char_index};
    }

    error semantic_error(std::string_view message, size_t line_number, size_t char_index)
    {
        std::string error_message("Semantic error: ");
        error_message += message;
        return {std::move(error_message), line_number, char_index};
    }

    error undeclared_error(std::string_view undeclared, size_t line_number, size_t char_index)
    {
        std::string message("Undeclared identifier '");
        message += undeclared;
        message += "'";
        return {std::move(message), line_number, char_index};
    }

    error wrong_type_error(std::string_view source, std::string_view destination, bool lvalue,
                           size_t line_number, size_t char_index)
    {
        std::string message;
        if (lvalue)
        {
            message += "'";
            message += source;
            message += "' is not an lvalue";
        }
        else
        {
            message += "Can't convert '";
            message += source;
            message += "' to '";
            message += destination;
            message += "'";
        }
        return semantic_error(message, line_number, char_index);
    }

    void print_error(const error& err, const character_source_t& source)
    {
        fmt::print(stderr, "({}) {}\n", err.line_number() + 1, err.what());

        std::size_t char_index = 0;

        for (std::size_t line_number = 0; line_number < err.line_number(); ++char_index)
        {
            int c = source();
            if (c < 0)
            {
                return;
            }
            else if (c == '\n')
            {
                ++line_number;
            }
        }

        std::size_t index_in_line = err.char_index() - char_index;

        std::string line;
        for (std::size_t idx = 0;; ++idx)
        {
            int c = source();
            if (c < 0 || c == '\n' || c == '\r')
            {
                break;
            }
            line += char(c);
        }

        fmt::print(stderr, "{}\n", line);

        for (std::size_t idx = 0; idx < index_in_line; ++idx)
        {
            fmt::print(stderr, " ", line);
        }

        fmt::print(stderr, "^\n", line);
    }
} // namespace tone::core