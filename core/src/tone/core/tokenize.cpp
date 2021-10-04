#include "tone/core/tokenize.hpp"
#include "tone/core/errors.hpp"

namespace tone::core {
    namespace {
        enum class character_category : std::uint8_t
        {
            eof,
            space,
            alphanum,
            punct,
        };

        character_category get_character_type(character_t c)
        {
            if (c < 0)
                return character_category::eof;
            if (std::isspace(c))
                return character_category::space;
            if (std::isalpha(c) || std::isdigit(c) || c == '_')
                return character_category::alphanum;
            return character_category::punct;
        }

        token fetch_word(push_back_stream& stream)
        {
            auto line_number = stream.line_number();
            auto char_index = stream.char_index();

            std::string word;

            auto c = stream();

            bool is_number = std::isdigit(c);

            do
            {
                word.push_back(char(c));
                c = stream();
            } while (get_character_type(c) == character_category::alphanum ||
                     (is_number && c == '.'));
            stream.push_back(c);

            if (auto t = get_keyword(word))
            {
                // Special handling for constants
                if (t == reserved_token::kw_true)
                {
                    return {true, line_number, char_index};
                }
                else if (t == reserved_token::kw_false)
                {
                    return {false, line_number, char_index};
                }
                else if (t == reserved_token::kw_null)
                {
                    return {null{}, line_number, char_index};
                }
                return {*t, line_number, char_index};
            }
            else
            {
                if (std::isdigit(word.front()))
                {
                    char* endptr;
                    double num = strtol(word.c_str(), &endptr, 0);
                    if (*endptr != 0)
                    {
                        num = strtod(word.c_str(), &endptr);
                        if (*endptr != 0)
                        {
                            auto remaining = word.size() - (endptr - word.c_str());
                            throw unexpected_error(std::string(1, char(*endptr)),
                                                   stream.line_number(),
                                                   stream.char_index() - remaining);
                        }
                    }
                    return {num, line_number, char_index};
                }
                else
                {
                    return {identifier{std::move(word)}, line_number, char_index};
                }
            }
        }

        token fetch_operator(push_back_stream& stream)
        {
            auto line_number = stream.line_number();
            auto char_index = stream.line_number();

            if (auto t = get_operator(stream))
            {
                return {*t, line_number, char_index};
            }
            else
            {
                std::string unexpected;
                auto err_line_number = stream.line_number();
                auto err_char_index = stream.char_index();
                for (auto c = stream(); get_character_type(c) == character_category::punct;
                     c = stream())
                    unexpected.push_back(char(c));
                throw unexpected_error(unexpected, err_line_number, err_char_index);
            }
        }

        token fetch_string(push_back_stream& stream)
        {
            auto line_number = stream.line_number();
            auto char_index = stream.char_index();

            std::string str;

            bool escaped = false;

            auto c = stream();

            for (; get_character_type(c) != character_category::eof; c = stream())
            {
                if (c == '\\')
                {
                    escaped = true;
                }
                else
                {
                    if (escaped)
                    {
                        if (c == 't')
                            str.push_back('\t');
                        else if (c == 'n')
                            str.push_back('\n');
                        else if (c == 'r')
                            str.push_back('\r');
                        else if (c == '0')
                            str.push_back('\0');
                        else
                            str.push_back(char(c));
                        escaped = false;
                    }
                    else
                    {
                        if (c == '\t' || c == '\n' || c == '\r')
                        {
                            stream.push_back(c);
                            throw parsing_error("Unclosed string", stream.line_number(),
                                                stream.char_index());
                        }
                        else if (c == '"')
                        {
                            return {std::move(str), line_number, char_index};
                        }
                        else
                        {
                            str.push_back(char(c));
                        }
                    }
                }
            }
            stream.push_back(c);
            throw parsing_error("Unclosed string", stream.line_number(), stream.char_index());
        }

        void skip_line_comment(push_back_stream& stream)
        {
            character_t c;
            do
            {
                c = stream();
            } while (c != '\n' && get_character_type(c) != character_category::eof);
            if (c != '\n')
            {
                stream.push_back(c);
            }
        }

    }// namespace
    token tokenize(push_back_stream& stream)
    {
        for (;;)
        {
            auto line_number = stream.line_number();
            auto char_index = stream.char_index();

            auto c = stream();

            switch (get_character_type(c))
            {
                case character_category::eof:
                    return {eof{}, line_number, char_index};
                case character_category::space:
                    continue;
                case character_category::alphanum:
                    stream.push_back(c);
                    return fetch_word(stream);
                case character_category::punct:
                    switch (c)
                    {
                        case '"':
                            return fetch_string(stream);
                        case '#':
                            skip_line_comment(stream);
                            continue;
                        default:
                            stream.push_back(c);
                            return fetch_operator(stream);
                    }
            }
        }
    }
}// namespace tone::core