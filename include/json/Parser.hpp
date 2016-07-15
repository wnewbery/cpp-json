#pragma once
#include "Token.hpp"
#include <stdexcept>
#include <sstream>
#include <cassert>
#include <cstring>
namespace json
{
    class ParseError : public std::runtime_error
    {
    public:
        static std::string full_msg(int line, int pos, const std::string &msg)
        {
            std::stringstream ss;
            ss << line << ':' << pos << ' ' << msg;
            return ss.str();
        }

        ParseError(int line, int pos, const std::string &msg)
            : std::runtime_error(full_msg(line, pos, msg))
        {}
        ParseError(const std::string &msg) : std::runtime_error(msg) {}
    };
    /**Parses JSON input as a sequence of tokens.
     * Does not keep track of previous tokens, so does no error checking involving such things.
     */
    class Parser
    {
    public:
        Parser(const char *begin, const char *end)
            : begin(begin), p(begin), end(end)
            , line_start(begin), line_num(1)
        {}

        /**Parse the next token in the input.*/
        Token next()
        {
            if (p > end) parse_error("Unexpected end of input");
            skip_ws();
            if (p == end) return ++p, Token::END;
            switch (*p)
            {
            case '[': return ++p, Token::ARR_START;
            case ']': return ++p, Token::ARR_END;
            case '{': return ++p, Token::OBJ_START;
            case '}': return ++p, Token::OBJ_END;
            case ':': return ++p, Token::KEY_SEP;
            case ',': return ++p, Token::ELEMENT_SEP;
            case 't': return consume_str("true"), Token::TRUE_VAL;
            case 'f': return consume_str("false"), Token::FALSE_VAL;
            case 'n': return consume_str("null"), Token::NULL_VAL;
            case '"': return parse_str();
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '-':
                return parse_num();
            default: parse_error("Unexpected content");
            }
        }
    private:
        const char *begin, *p, *end;
        const char *line_start;
        int line_num;

        [[noreturn]] void parse_error(const std::string &msg)
        {
            throw ParseError(line_num, (int)(p - line_start), msg);
        }

        template<size_t N>
        void consume_str(const char (&str)[N])
        {
            assert(p[0] == str[0]);
            if (p + N - 1<= end && memcmp(p + 1, str + 1, N - 2) == 0)
            {
                p += N - 1;
            }
            else parse_error(std::string("Expected ") + str);
        }

        void skip_ws()
        {
            while (p < end)
            {
                switch (*p)
                {
                case '\r':
                    ++p;
                    if (p < end && *p == '\n') ++p; //count \r\n as a single newline
                    ++line_num;
                    line_start = p;
                    break;
                case '\n':
                    ++p;
                    ++line_num;
                    line_start = p;
                    break;
                case ' ':
                case '\t':
                    ++p;
                    break;
                default:
                    return;
                }
            }
        }
    
        static bool is_digit(char c)
        {
            switch (c)
            {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                return true;
            default:
                return false;
            }
        }
        Token parse_num()
        {
            assert(is_digit(*p) || *p == '-');
            auto first = p;
            if (*p == '-') ++p;
            while (p < end && is_digit(*p)) ++p;
            if (p < end && *p == '.')
            {
                ++p;
                while (p < end && is_digit(*p)) ++p;
            }
            return { Token::NUMBER, { first, p }};
        }

        //TODO: Does not handle escape codes, or any illegal bytes, just goes to next "
        Token parse_str()
        {
            assert(*p == '"');
            ++p;
            auto first = p;
            while (p < end && *p != '"') ++p;
            if (p == end) parse_error("End of string not found");
            ++p; // "
            return { Token::STRING, { first, p - 1 }};
        }
    };
}
