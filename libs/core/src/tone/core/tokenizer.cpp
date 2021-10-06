#include "tone/core/tokenizer.hpp"

#include "tone/core/tokenize.hpp"

#include <utility>

namespace tone::core {
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// `tokenizer::iterator` class
    ////////////////////////////////////////////////////////////////////////////////////////////////

    tokenizer::iterator::iterator(push_back_stream& strm)
            : _strm(strm)
    {
        _tok = tokenize(_strm);
    }
    tokenizer::iterator::iterator(push_back_stream& strm, token tok)
            : _strm(strm)
            , _tok(std::move(tok))
    {
    }

    tokenizer::iterator& tokenizer::iterator::operator++()
    {
        _tok = tokenize(_strm);
        return *this;
    }

    const tokenizer::iterator tokenizer::iterator::operator++(int)
    {
        tokenizer::iterator retval = *this;
        ++(*this);
        return retval;
    }
    bool tokenizer::iterator::operator==(tokenizer::iterator other) const
    {
        return _tok == other._tok;
    }
    bool tokenizer::iterator::operator!=(tokenizer::iterator other) const
    {
        return !(*this == other);
    }
    token tokenizer::iterator::operator*()
    {
        return _tok;
    }
    tokenizer::iterator::operator bool() const
    {
        return !_tok.is_eof();
    }
    const token* tokenizer::iterator::operator->() const
    {
        return &_tok;
    }
    token* tokenizer::iterator::operator->()
    {
        return &_tok;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// `tokenizer` class
    ////////////////////////////////////////////////////////////////////////////////////////////////

    tokenizer::tokenizer(push_back_stream strm)
        : _strm(std::move(strm))
    {}

    tokenizer::iterator tokenizer::begin()
    {
        return iterator(_strm);
    }
    tokenizer::iterator tokenizer::end()
    {
        return iterator(_strm, token());
    }
}