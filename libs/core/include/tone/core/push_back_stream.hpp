#pragma once

#include "tone/core/character.hpp"

#include <stack>

namespace tone::core {
    class push_back_stream
    {
    public:
        explicit push_back_stream(const character_source_t& input);

        character_t operator()();

        void push_back(character_t c);

        [[nodiscard]] std::size_t line_number() const;
        [[nodiscard]] std::size_t char_index() const;
    private:
        const character_source_t& _input;
        std::stack<character_t> _stack;
        std::size_t _line_num;
        std::size_t _char_idx;
    };
}