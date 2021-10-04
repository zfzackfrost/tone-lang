#pragma once

#include "tone/core/character.hpp"

#include <exception>
#include <ostream>
#include <string>
#include <string_view>


namespace tone::core {
    class error : public std::exception
    {
    public:
        error(std::string message, std::size_t line_number, std::size_t char_index);

        [[nodiscard]] const char* what() const noexcept override;
        [[nodiscard]] std::size_t line_number() const;
        [[nodiscard]] std::size_t char_index() const;

    private:
        std::string _message;
        std::size_t _line_number;
        std::size_t _char_index;
    };

    error parsing_error(const char* message, std::size_t line_number, std::size_t char_index);
    error unexpected_error(std::string_view unexpected, std::size_t line_number,
                           std::size_t char_index);
    void print_error(const error& err, const character_source_t& source);
}// namespace tone::core