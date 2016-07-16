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

        /**True if type is a value, or the start of a value.
         * ARR_START, OBJ_START, NUMBER, STRING, TRUE_VAL, FALSE_VAL, NULL_VAL
         */
        bool is_val()const
        {
            switch (type)
            {
            case ARR_START:
            case OBJ_START:
            case NUMBER:
            case STRING:
            case TRUE_VAL:
            case FALSE_VAL:
            case NULL_VAL:
                return true;
            default:
                return false;
            }
        }
    };
}
