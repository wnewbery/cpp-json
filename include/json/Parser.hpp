#pragma once
#include "Token.hpp"
#include <stdexcept>
#include <sstream>
#include <limits>
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
                return do_next_number();
            default: parse_error("Unexpected content");
            }
        }

        /**If the next token would be ARR_END, consume it and return true.*/
        bool try_next_arr_end()
        {
            if (p > end) parse_error("Unexpected end of input");
            skip_ws();
            if (p < end && *p == ']')
            {
                ++p;
                return true;
            }
            else return false;
        }
        /**Skip the rest of a quoted string. */
        void skip_remaining_str()
        {
            while (p < end)
            {
                if (*p == '"')
                {
                    ++p;
                    return;
                }
                else if (*p == '\\')
                {
                    ++p;
                    if (p == end) break;
                    ++p;
                }
                else ++p;
            }
            parse_error("Failed to find end of string");
        }
        /**Read the next string, but discard the contents. */
        void skip_next_str()
        {
            skip_ws();
            if (p >= end || *p != '"') parse_error("Expected string");
            skip_remaining_str();
        }

        /**Read the next string. */
        std::string next_str()
        {
            std::string tmp;
            next_str(&tmp);
            return tmp;
        }
        void next_str(std::string *out)
        {
            skip_ws();
            if (p >= end || *p != '"') parse_error("Expected string");
            parse_str(out);
        }
        template<typename T> T next_int()
        {
            skip_ws();
            if (p >= end) parse_error("Expected int");
            return do_next_int<T>();
        }
        template<typename T> T next_uint()
        {
            skip_ws();
            if (p >= end) parse_error("Expected unsigned int");
            return do_next_positive_int<T>();
        }

        double next_double()
        {
            skip_ws();
            if (p >= end) parse_error("Expected int");
            auto start = p;
            if (*p == '-') ++p;
            return complete_next_float(start);
        }


        //Objects
        void next_obj_start() { next_chr<'{'>(); }
        /**Next ':' key-value seperator.*/
        void next_key_sep()
        {
            skip_ws();
            if (p < end && *p == ':')
            {
                ++p;
            }
            else parse_error("Expected ':'");
        }
        /**true if next is ',', false if '}', else error.*/
        bool next_obj_el()
        {
            skip_ws();
            if (p < end)
            {
                if (*p == ',')
                {
                    ++p;
                    return true;
                }
                if (*p == '}')
                {
                    ++p;
                    return false;
                }
            }
            parse_error("Expected ',' or '}' after object value");
        }
        /**If the next token would be OBJ_END, consume it and return true.*/
        bool try_next_obj_end()
        {
            skip_ws();
            if (p < end)
            {
                if (*p == '}')
                {
                    ++p;
                    return true;
                }
                else return false;
            }
            parse_error("Unexpected end of input");
        }
    private:
        const char *begin, *p, *end;
        const char *line_start;
        int line_num;

        [[noreturn]] void parse_error(const std::string &msg)
        {
            throw ParseError(line_num, (int)(p - line_start), msg);
        }

        template<char c>
        void next_chr()
        {
            skip_ws();
            if (p < end && *p == c)
            {
                ++p;
            }
            else parse_error(std::string("Expected ") + c);
        }

        template<size_t N>
        void consume_str(const char (&str)[N])
        {
            assert(p[0] == str[0]);
            if (p + N - 1 <= end && memcmp(p + 1, str + 1, N - 2) == 0)
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

        template<typename T>
        T do_next_int()
        {
            assert(p < end);
            if (*p == '-')
            {
                ++p;
                if (p == end) parse_error("Unexpected end");
                return do_next_negative_int<T>();
            }
            else return do_next_positive_int<T>();
        }

        template<typename T>
        T do_next_positive_int()
        {
            typedef std::numeric_limits<T> limits;
            assert(p < end);
            if (!is_digit(*p)) parse_error("Expected int");
            T x = 0;
            do
            {
                int y = *p - '0';
                if (x > limits::max() / 10) parse_error("Overflow");
                if (x == limits::max() / 10 && y > (int)(limits::max() % 10)) parse_error("Overflow");
                x = (T)(x * 10 + y);
                ++p;
            }
            while (p < end && is_digit(*p));
            return x;
        }
        template<typename T>
        T do_next_negative_int()
        {
            typedef std::numeric_limits<T> limits;
            typedef typename std::make_unsigned<T>::type ut;
            assert(p < end);
            if (!is_digit(*p)) parse_error("Expected int");
            T x = 0;
            do
            {
                int y = *p - '0';
                if (x < limits::min() / 10) parse_error("Overflow");
                if (x == limits::min() / 10 && y > (int)(((ut)limits::min()) % 10)) parse_error("Overflow");
                x = (T)(x * 10 - y);
                ++p;
            }
            while (p < end && is_digit(*p));
            return x;
        }

        double complete_next_float(const char *start)
        {
            assert(p < end && start <= p);
            while(p < end && is_digit(*p)) ++p;
            if (p < end && *p == '.')
            {
                ++p;
                while (p < end && is_digit(*p)) ++p;
            }
            if (p < end && (*p == 'e' || *p == 'E'))
            {
                ++p;
                if (p < end && (*p == '+' || *p == '-'))
                {
                    ++p;
                    while (p < end && is_digit(*p)) ++p;
                }
            }
            //using stod for now
            std::string str(start, p);
            size_t count;
            try
            {
                auto x = std::stod(str, &count);
                if (count != str.size()) parse_error(str + " is not a valid number");
                return x;
            }
            catch (const std::exception &)
            {
                parse_error("Expected number");
            }
        }

        Token do_next_number()
        {
            auto start = p;
            long long x = do_next_int<long long>();
            if (p >= end || (*p != '.' && *p != 'e' && *p != 'E'))
            {
                return Token(x);
            }
            else
            {
                ++p;
                if (p >= end) parse_error("Unexpected end");
                return Token(complete_next_float(start));
            }
        }

        //TODO: Does not handle escape codes, or any illegal bytes, just goes to next "
        void parse_str(std::string *out)
        {
            assert(*p == '"');
            ++p;
            while (p < end && *p != '"')
            {
                if (*p != '\\')
                {
                    *out += *p;
                    ++p;
                }
                else
                {
                    if (++p == end) parse_error("End of string not found");
                    switch (*p)
                    {
                    case 'n': *out += '\n'; ++p; break;
                    case 'r': *out += '\r'; ++p; break;
                    case '"': *out += '\"'; ++p; break;
                    case 'b': *out += '\b'; ++p; break;
                    case 'f': *out += '\f'; ++p; break;
                    case 't': *out += '\t'; ++p; break;
                    case '\'': *out += '\''; ++p; break;
                    case '\\': *out += '\\'; ++p; break;
                    case 'u': decode_unicode(out); break;
                    }
                }
            }
            if (p == end) parse_error("End of string not found");
            ++p; // "
        }
        Token parse_str()
        {
            Token ret = Token::STRING;
            parse_str(&ret.str);
            return ret;
        }

        void decode_unicode(std::string *buf)
        {
            assert(*p == 'u');
            ++p;
            if (p + 4 > end) parse_error("Unexpected end");
            //this is a slight pain because JSON always uses UTF-16, even in its escape codes
            //so must deal with surrogate pairs to get a code point, then convert to utf-8
            unsigned high = decode_unicode_utf16_el();
            if (high <= 0xD7FF || high >= 0xE000) //single element
            {
                cp_to_utf8(high, buf);
            }
            else //surrogate pair
            {
                if (high >= 0xDC00) parse_error("Unexpected low surrogate");

                if (p + 6 > end) parse_error("Unexpected end looking for second surrogate");
                if (p[0] != '\\' || p[1] != 'u') parse_error("Expected \\uxxxx for second surrogate");
                p += 2;
                unsigned low = decode_unicode_utf16_el();
                if (low < 0xDC00 || low > 0xDFFF) parse_error("Expected low surrogate");
                unsigned cp = ((high - 0xD800) << 10) + (low - 0xDC00) + 0x10000;
                cp_to_utf8(cp, buf);
            }
        }
        unsigned decode_unicode_utf16_el()
        {
            unsigned cp =
                (decode_hex(p[0]) << 12) |
                (decode_hex(p[1]) <<  8) |
                (decode_hex(p[2]) <<  4) |
                (decode_hex(p[3]) <<  0);
            p += 4;
            return cp;
        }
        unsigned decode_hex(char c)
        {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            parse_error("Invalid hex digit");
        }
        void cp_to_utf8(unsigned cp, std::string *buf)
        {
            if (cp < 0x80) //0xxxxxxx
            {
                *buf += (char)cp;
            }
            else if (cp < 0x800) //110xxxxx 10xxxxxx
            {
                *buf += (char)(0b11000000 | (cp >> 6));
                *buf += (char)(0b10000000 | ((cp >> 0) & 0b00111111));
            }
            else if (cp < 0x10000) //1110xxxx 10xxxxxx 10xxxxxx
            {
                *buf += (char)(0b11100000 | (cp >> 12));
                *buf += (char)(0b10000000 | ((cp >> 6) & 0b00111111));
                *buf += (char)(0b10000000 | ((cp >> 0) & 0b00111111));
            }
            else //11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            {
                assert(cp < 0x110000); // The maximun allowed surrogate pairs give 0x10FFFF so this cant happen
                *buf += (char)(0b11110000 | (cp >> 18));
                *buf += (char)(0b10000000 | ((cp >> 12) & 0b00111111));
                *buf += (char)(0b10000000 | ((cp >>  6) & 0b00111111));
                *buf += (char)(0b10000000 | ((cp >>  0) & 0b00111111));
            }
        }
    };
}
