#include <boost/test/unit_test.hpp>
#include "Parser.hpp"

using namespace json;
BOOST_AUTO_TEST_SUITE(TestParser)

Token parse_single(const std::string &str)
{
    Parser parser(str.data(), str.data() + str.size());
    return parser.next();
}

BOOST_AUTO_TEST_CASE(test)
{
    BOOST_CHECK_EQUAL(Token::END, parse_single("").type);
    BOOST_CHECK_EQUAL(Token::ARR_START, parse_single("[").type);
    BOOST_CHECK_EQUAL(Token::ARR_END, parse_single("]").type);
    BOOST_CHECK_EQUAL(Token::OBJ_START, parse_single("{").type);
    BOOST_CHECK_EQUAL(Token::OBJ_END, parse_single("}").type);
    BOOST_CHECK_EQUAL(Token::KEY_SEP, parse_single(":").type);
    BOOST_CHECK_EQUAL(Token::ELEMENT_SEP, parse_single(",").type);
    BOOST_CHECK_EQUAL(Token::TRUE_VAL, parse_single("true").type);
    BOOST_CHECK_EQUAL(Token::FALSE_VAL, parse_single("false").type);
    BOOST_CHECK_EQUAL(Token::NULL_VAL, parse_single("null").type);
    BOOST_CHECK_THROW(parse_single("x"), ParseError);
    BOOST_CHECK_EQUAL(Token::NULL_VAL, parse_single("  \r\n\t\n   null").type);

    {
        std::string str = "";
        Parser parser(str.data(), str.data() + str.size());
        BOOST_CHECK_EQUAL(Token::END, parser.next().type);
        BOOST_CHECK_THROW(parser.next(), ParseError);
    }
    {
        auto tok = parse_single("-500.5");
        BOOST_CHECK_EQUAL(Token::NUMBER, tok.type);
        BOOST_CHECK_EQUAL("-500.5", tok.str);
    }
    {
        auto tok = parse_single("\"Hello World\"");
        BOOST_CHECK_EQUAL(Token::STRING, tok.type);
        BOOST_CHECK_EQUAL("Hello World", tok.str);
    }
}

BOOST_AUTO_TEST_SUITE_END()
