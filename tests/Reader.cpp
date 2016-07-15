#include <boost/test/unit_test.hpp>
#include "Reader.hpp"
#include <unordered_map>
#include <vector>

using namespace json;
struct MyType
{
    int x, y, z;
};

inline void read_json(Parser &parser, MyType *val)
{
    static const auto reader = ObjectFieldReader<MyType>().
        JSON_READER_FIELD(MyType, x).
        JSON_READER_FIELD(MyType, y).
        JSON_READER_FIELD(MyType, z);

    reader.read(parser, val);
}

BOOST_AUTO_TEST_SUITE(TestParser)

BOOST_AUTO_TEST_CASE(primitives)
{
    BOOST_CHECK_EQUAL(100, read_json<int>("100"));
    BOOST_CHECK_EQUAL(100.5, read_json<float>("100.5"));
    BOOST_CHECK_EQUAL(true, read_json<bool>("true"));
    BOOST_CHECK_EQUAL(false, read_json<bool>("false"));
    BOOST_CHECK_EQUAL("Hello World", read_json<std::string>("\"Hello World\""));
}

BOOST_AUTO_TEST_CASE(arr)
{
    const int numbers[] = {10, 15, -5, 1};
    std::vector<int> parsed;
    read_json("[10, 15, -5,1]", &parsed);
    BOOST_CHECK_EQUAL_COLLECTIONS(
        numbers, numbers + sizeof(numbers) / sizeof(numbers[0]),
        parsed.begin(), parsed.end());
}

BOOST_AUTO_TEST_CASE(map)
{
    std::unordered_map<std::string, int> parsed;
    read_json("{\"b\": 5, \"a\": 10}", &parsed);
    BOOST_CHECK_EQUAL(2, parsed.size());
    BOOST_CHECK_EQUAL(10, parsed["a"]);
    BOOST_CHECK_EQUAL(5, parsed["b"]);
}

BOOST_AUTO_TEST_CASE(obj)
{
    auto obj = read_json<MyType>("{\"x\": 0, \"y\": 10, \"z\": -5}");
    BOOST_CHECK_EQUAL(0, obj.x);
    BOOST_CHECK_EQUAL(10, obj.y);
    BOOST_CHECK_EQUAL(-5, obj.z);


    auto vec = read_json<std::vector<MyType>>(
        "[{\"x\": 0, \"y\": 10, \"z\": -5}, {\"x\": 1, \"y\": 2, \"z\": 3}]");
    BOOST_CHECK_EQUAL(2, vec.size());
    if (2 == vec.size())
    {
        BOOST_CHECK_EQUAL(0, vec[0].x);
        BOOST_CHECK_EQUAL(10, vec[0].y);
        BOOST_CHECK_EQUAL(-5, vec[0].z);

        BOOST_CHECK_EQUAL(1, vec[1].x);
        BOOST_CHECK_EQUAL(2, vec[1].y);
        BOOST_CHECK_EQUAL(3, vec[1].z);
    }
}

BOOST_AUTO_TEST_SUITE_END()
