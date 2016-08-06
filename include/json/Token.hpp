#pragma once
#include <string>
namespace json
{
    /**A token read by json::Parser.*/
    struct Token
    {
        enum Type
        {
            /** End of input. */
            END,
            /** '[' */
            ARR_START,
            /** ']' */
            ARR_END,
            /** '{' */
            OBJ_START,
            /** '}' */
            OBJ_END,
            /** ':' */
            KEY_SEP,
            /** ',' */
            ELEMENT_SEP,
            /**Integer number.*/
            INTEGER,
            /**Number, string stored in str.*/
            NUMBER,
            /**String, value (Without quotes) stored in str.*/
            STRING,
            /**Boolean true value.*/
            TRUE_VAL,
            /**Boolean false value.*/
            FALSE_VAL,
            /**Nul value.*/
            NULL_VAL
        };

        Type type;
        std::string str;
        union
        {
            long long val_int;
            double val_num;
        };

        Token() : type(END), str() {}
        Token(Type type) : type(type), str() {}
        Token(std::string &&str) : type(STRING), str(std::move(str)) {}
        Token(long long x) : type(INTEGER), val_int(x) {}
        Token(double x) : type(NUMBER), val_num(x) {}
    };
}
