#include <boost/test/unit_test.hpp>
#include "Parser.hpp"
#include <sstream>
#include <iomanip>

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
    BOOST_CHECK_THROW(parse_single("trux"), ParseError);
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
        BOOST_CHECK_EQUAL(-500.5, tok.val_num);
    }
    {
        auto tok = parse_single("\"Hello World\"");
        BOOST_CHECK_EQUAL(Token::STRING, tok.type);
        BOOST_CHECK_EQUAL("Hello World", tok.str);
    }
}

std::string str_to_bytestr(const std::string &str)
{
    std::stringstream ss;
    ss << std::setfill('0') << std::hex << std::right;
    ss << '"';
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (i != 0) ss << ' ';
        ss << std::setw(2) << ((unsigned)(unsigned char)str[i]);
    }
    ss << '"';
    return ss.str();
}
boost::test_tools::predicate_result byte_cmp(const std::string &lhs, const std::string &rhs)
{
    if (lhs == rhs) return true;
    boost::test_tools::predicate_result res = false;
    res.message() << str_to_bytestr(lhs) + " != " + str_to_bytestr(rhs);
    return res;
}
#define CHECK_BYTES_EQUAL(a, b) BOOST_CHECK(byte_cmp(a, b))


BOOST_AUTO_TEST_CASE(strings)
{
    BOOST_CHECK_EQUAL("Hello World", parse_single("\"Hello World\"").str);
    BOOST_CHECK_EQUAL("Escaped \n \r \" ' \b \f \t \\ ",
        parse_single("\"Escaped \\n \\r \\\" \\' \\b \\f \\t \\\\ \"").str);

    CHECK_BYTES_EQUAL(std::string("\0", 1), parse_single("\"\\u0000\"").str);
    CHECK_BYTES_EQUAL(u8"\u00A3", parse_single("\"\\u00A3\"").str);
    CHECK_BYTES_EQUAL(u8"\u00A3", parse_single("\"\\u00a3\"").str);
    CHECK_BYTES_EQUAL(u8"\u00A3F", parse_single("\"\\u00a3F\"").str);


    CHECK_BYTES_EQUAL(u8"\u2211", parse_single("\"\\u2211\"").str);
    CHECK_BYTES_EQUAL(u8"\U0010FFFF", parse_single("\"\\uDBFF\\uDFFF\"").str);

    BOOST_CHECK_THROW(parse_single("\"\\uDBFF\""), ParseError); // missing low
    BOOST_CHECK_THROW(parse_single("\"\\uDBFF\\u0000\""), ParseError); // invalid low
    BOOST_CHECK_THROW(parse_single("\"\\uDBFF\\uE000\""), ParseError); // invalid low
    BOOST_CHECK_THROW(parse_single("\"\\uDFFF\""), ParseError); // unexpected low
    BOOST_CHECK_THROW(parse_single("\"\\u\""), ParseError); // missing values
    BOOST_CHECK_THROW(parse_single("\"\\u800\""), ParseError); // missing values
    BOOST_CHECK_THROW(parse_single("\"\\u800G\""), ParseError); // invalid values
    BOOST_CHECK_THROW(parse_single("\"\\uXXXX\""), ParseError); // invalid values
    BOOST_CHECK_THROW(parse_single("\"\\uDBFF\\u\""), ParseError); // missing low values
    BOOST_CHECK_THROW(parse_single("\"\\uDBFF\\u0\""), ParseError); // missing low values
    BOOST_CHECK_THROW(parse_single("\"\\uDBFF\\u00\""), ParseError); // missing low values
    BOOST_CHECK_THROW(parse_single("\"\\uDBFF\\u000\""), ParseError); // missing low values

}

BOOST_AUTO_TEST_SUITE_END()
