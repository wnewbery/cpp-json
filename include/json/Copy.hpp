#pragma once
#include "Parser.hpp"
#include "Writer.hpp"

namespace json
{
    /**Copy the next value in parser, including objects and arrays, to writer.*/
    inline void copy(Writer &writer, Parser &parser);


    namespace detail
    {
        void copy_arr(Writer &writer, Parser &parser)
        {
            writer.start_arr();
            if (parser.try_next_arr_end()) return writer.end_arr();
            Token tok;
            do
            {
                copy(writer, parser);
                tok = parser.next();
            } while (tok.type == Token::ELEMENT_SEP);
            if (tok.type != Token::ARR_END) throw ParseError("Expected array end");
            writer.end_arr();
        }
        void copy_obj(Writer &writer, Parser &parser)
        {
            writer.start_obj();
            if (parser.try_next_obj_end()) return writer.end_obj();
            Token tok;
            do
            {
                tok = parser.next();
                if (tok.type != Token::STRING) throw ParseError("Expected object key");
                writer.key(tok.str);

                tok = parser.next();
                if (tok.type != Token::KEY_SEP) throw ParseError("Expected ':' object key-value seperator");

                copy(writer, parser);
                tok = parser.next();
            } while (tok.type == Token::ELEMENT_SEP);
            if (tok.type != Token::OBJ_END) throw ParseError("Expected object end");
            writer.end_obj();
        }
    }
    inline void copy(Writer &writer, Parser &parser)
    {
        auto tok = parser.next();
        switch (tok.type)
        {
        case Token::ARR_START: detail::copy_arr(writer, parser); return;
        case Token::OBJ_START: detail::copy_obj(writer, parser); return;
        case Token::INTEGER: writer.value(tok.val_int); return;
        case Token::NUMBER: writer.value(tok.val_num); return;
        case Token::STRING: writer.value(tok.str); return;
        case Token::TRUE_VAL: writer.value(true); return;
        case Token::FALSE_VAL: writer.value(false); return;
        case Token::NULL_VAL: writer.null(); return;
        default: throw ParseError("Unexpected token");
        }
    }
}
