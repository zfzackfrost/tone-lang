#include "tone/core/push_back_stream.hpp"

namespace tone::core {

    push_back_stream::push_back_stream(const character_source_t& input)
        : _input(input)
        , _line_num(0)
        , _char_idx(0)
    {}

    character_t push_back_stream::operator()()
    {
        character_t ret;
        if (_stack.empty())
            ret = _input();
        else
        {
            ret = _stack.top();
            _stack.pop();
        }
        if (ret == '\n')
            ++_line_num;

        ++_char_idx;
        return ret;
    }

    void push_back_stream::push_back(character_t c)
    {
        _stack.push(c);
        if (c == '\n')
            --_line_num;
        --_char_idx;
    }

    std::size_t push_back_stream::line_number() const
    {
        return _line_num;
    }

    std::size_t push_back_stream::char_index() const
    {
        return _char_idx;
    }
}// namespace tone::core