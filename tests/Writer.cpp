#include <boost/test/unit_test.hpp>
#include "Writer.hpp"
#include <unordered_map>
#include <vector>

using namespace json;

BOOST_AUTO_TEST_SUITE(TestWriter)

template<typename T> std::string write_single(const T &val)
{
    Writer writer;
    writer.value(val);
    return writer.str();
}
BOOST_AUTO_TEST_CASE(single)
{
    int *ptr = nullptr;
    int x = 5;
    int *ptr2 = &x;

    BOOST_CHECK_EQUAL("true", write_single(true));
    BOOST_CHECK_EQUAL("false", write_single(false));
    BOOST_CHECK_EQUAL("\"Hello World\"", write_single("Hello World"));
    BOOST_CHECK_EQUAL("100", write_single(100));
    BOOST_CHECK_EQUAL("100.5", write_single(100.5));
    BOOST_CHECK_EQUAL("null", write_single(ptr));
    BOOST_CHECK_EQUAL("5", write_single(ptr2));


    BOOST_CHECK_EQUAL("\"Escaped \\b\\r\\n\\t\\\"\\\\n\"", write_single("Escaped \b\r\n\t\"\\n"));
}

BOOST_AUTO_TEST_CASE(arr)
{
    Writer writer;
    writer.start_arr();
    writer.value(1);
    writer.value(5);
    writer.end_arr();
    BOOST_CHECK_EQUAL("[1,5]", writer.str());
}

BOOST_AUTO_TEST_CASE(obj)
{
    Writer writer;
    writer.start_obj();
    writer.key("a");
    writer.value(1);
    writer.key("b");
    writer.value(5);
    writer.prop("c", "x");
    writer.end_obj();
    BOOST_CHECK_EQUAL("{\"a\":1,\"b\":5,\"c\":\"x\"}", writer.str());
}

BOOST_AUTO_TEST_SUITE_END()
