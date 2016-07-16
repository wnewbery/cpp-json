#include <boost/test/unit_test.hpp>
#include "Copy.hpp"
#include <unordered_map>
#include <vector>

using namespace json;

BOOST_AUTO_TEST_SUITE(TestCopy)

std::string do_copy(const std::string &src)
{
    Writer writer;
    Parser parser(src.data(), src.data() + src.size());
    copy(writer, parser);
    return std::move(writer.str());
}

BOOST_AUTO_TEST_CASE(primitives)
{
    BOOST_CHECK_EQUAL("5", do_copy("5"));
    BOOST_CHECK_EQUAL("5.5", do_copy("5.5"));
    BOOST_CHECK_EQUAL("true", do_copy("true"));
    BOOST_CHECK_EQUAL("false", do_copy("false"));
    BOOST_CHECK_EQUAL("null", do_copy("null"));

    BOOST_CHECK_EQUAL("[]", do_copy("[]"));
    BOOST_CHECK_EQUAL("[5]", do_copy("[5]"));
    BOOST_CHECK_EQUAL("[5,6]", do_copy("[5, 6]"));

    BOOST_CHECK_EQUAL("{}", do_copy("{}"));
    BOOST_CHECK_EQUAL("{\"a\":true}", do_copy("{\"a\": true}"));

    BOOST_CHECK_THROW(do_copy(""), ParseError);
    BOOST_CHECK_THROW(do_copy("invalid"), ParseError);
    BOOST_CHECK_THROW(do_copy(","), ParseError);
    BOOST_CHECK_THROW(do_copy("}"), ParseError);

    BOOST_CHECK_THROW(do_copy("]"), ParseError);
    BOOST_CHECK_THROW(do_copy("["), ParseError);
    BOOST_CHECK_THROW(do_copy("[5"), ParseError);
    BOOST_CHECK_THROW(do_copy("[,"), ParseError);
    BOOST_CHECK_THROW(do_copy("[5,"), ParseError);
    BOOST_CHECK_THROW(do_copy("[5,]"), ParseError);

    BOOST_CHECK_THROW(do_copy("{"), ParseError);
    BOOST_CHECK_THROW(do_copy("{\"a\""), ParseError);
    BOOST_CHECK_THROW(do_copy("{\"a\":"), ParseError);
    BOOST_CHECK_THROW(do_copy("{\"a\": true"), ParseError);
    BOOST_CHECK_THROW(do_copy("{\"a\": true, "), ParseError);

}


BOOST_AUTO_TEST_SUITE_END()
