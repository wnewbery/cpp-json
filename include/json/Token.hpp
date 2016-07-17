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

        Token() : type(END), str() {}
        Token(Type type) : type(type), str() {}
        Token(Type type, std::string &&str) : type(type), str(std::move(str)) {}
    };
}
