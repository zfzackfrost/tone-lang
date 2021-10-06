#pragma once

#include "tone/core/tokens.hpp"

namespace tone::core {
    class tokenizer
    {
    public:
        class iterator
        {
        public:

            explicit iterator(push_back_stream& strm);
            explicit iterator(push_back_stream& strm, token tok);

            iterator& operator++();
            const iterator operator++(int);
            bool operator==(iterator other) const;
            bool operator!=(iterator other) const;
            token operator*();
            const token* operator->() const;
            token* operator->();

            operator bool() const;

            // iterator traits
            using value_type = token;
            using reference = const token&;
            using iterator_category = std::forward_iterator_tag;

        private:
            token _tok;
            push_back_stream& _strm;
        };

        explicit tokenizer(push_back_stream strm);

        iterator begin();
        iterator end();
    private:
        push_back_stream _strm;
    };
    using token_iterator = tokenizer::iterator;
} // namespace tone::core