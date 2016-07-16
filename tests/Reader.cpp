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
        add<decltype(MyType::x), &MyType::x>("x").
        add<decltype(MyType::y), &MyType::y>("y").
        add<decltype(MyType::z), &MyType::z>("z");

    reader.read(parser, val);
}

//more complex types
template<typename T>
struct ResponsePage
{
    size_t first;
    size_t total;
    std::vector<T> data;
};
template<typename T> void read_json(Parser &parser, ResponsePage<T> *val)
{
    static const auto reader = ObjectFieldReader<ResponsePage<T>>().
        add<decltype(ResponsePage<T>::first), &ResponsePage<T>::first>("first").
        add<decltype(ResponsePage<T>::total), &ResponsePage<T>::total>("total").
        add<decltype(ResponsePage<T>::data), &ResponsePage<T>::data>("data");
    reader.read(parser, val);
}

struct Comment
{
    std::string author;
    std::string text;
};
inline void read_json(Parser &parser, Comment *val)
{
    static const auto reader = ObjectFieldReader<Comment, IgnoreUnknown>().
        add<decltype(Comment::author), &Comment::author>("author").
        add<decltype(Comment::text), &Comment::text>("text");
    reader.read(parser, val);
}

BOOST_AUTO_TEST_SUITE(TestReader)

BOOST_AUTO_TEST_CASE(primitives)
{
    BOOST_CHECK_EQUAL(100, read_json<int>("100"));
    BOOST_CHECK_THROW(read_json<char>("500"), std::out_of_range);
    BOOST_CHECK_THROW(read_json<char>("50.5"), std::invalid_argument);
    BOOST_CHECK_THROW(read_json<int>("true"), ParseError);

    BOOST_CHECK_THROW(read_json<unsigned char>("500"), std::out_of_range);
    BOOST_CHECK_THROW(read_json<unsigned char>("-1"), std::out_of_range);
    BOOST_CHECK_THROW(read_json<unsigned char>("50.5"), std::invalid_argument);
    BOOST_CHECK_THROW(read_json<unsigned>("true"), ParseError);

    BOOST_CHECK_EQUAL(100.5, read_json<float>("100.5"));
    BOOST_CHECK_THROW(read_json<float>("true"), ParseError);


    BOOST_CHECK_EQUAL(true, read_json<bool>("true"));
    BOOST_CHECK_EQUAL(false, read_json<bool>("false"));
    BOOST_CHECK_THROW(read_json<bool>("5"), ParseError);

    BOOST_CHECK_EQUAL("Hello World", read_json<std::string>("\"Hello World\""));
}

BOOST_AUTO_TEST_CASE(arr)
{
    const int numbers[] = {10, 15, -5, 1};
    std::vector<int> parsed;
    BOOST_CHECK_NO_THROW(read_json("[10, 15, -5,1]", &parsed));
    BOOST_CHECK_EQUAL_COLLECTIONS(
        numbers, numbers + sizeof(numbers) / sizeof(numbers[0]),
        parsed.begin(), parsed.end());

    parsed.clear();
    BOOST_CHECK_NO_THROW(read_json("[]", &parsed));
    BOOST_CHECK(parsed.empty());

    parsed.clear();
    BOOST_CHECK_NO_THROW(read_json("[1]", &parsed));
    BOOST_CHECK_EQUAL(1, parsed.size());
    if (parsed.size() == 1) BOOST_CHECK_EQUAL(1, parsed[0]);


    BOOST_CHECK_THROW(read_json("5", &parsed), ParseError);
    BOOST_CHECK_THROW(read_json("[", &parsed), ParseError);
    BOOST_CHECK_THROW(read_json("[,", &parsed), ParseError);
    BOOST_CHECK_THROW(read_json("[5,", &parsed), ParseError);
    BOOST_CHECK_THROW(read_json("[5,]", &parsed), ParseError);
    BOOST_CHECK_THROW(read_json("[5}", &parsed), ParseError);

}

BOOST_AUTO_TEST_CASE(map)
{
    std::unordered_map<std::string, int> parsed;
    read_json("{\"b\": 5, \"a\": 10}", &parsed);
    BOOST_CHECK_EQUAL(2, parsed.size());
    BOOST_CHECK_EQUAL(10, parsed["a"]);
    BOOST_CHECK_EQUAL(5, parsed["b"]);

    parsed.clear();
    BOOST_CHECK_NO_THROW(read_json("{}", &parsed));
    BOOST_CHECK(parsed.empty());

    parsed.clear();
    BOOST_CHECK_NO_THROW(read_json("{\"a\": 6}", &parsed));
    BOOST_CHECK_EQUAL(6, parsed["a"]);

    BOOST_CHECK_THROW(read_json("5", &parsed), ParseError);
    BOOST_CHECK_THROW(read_json("{", &parsed), ParseError);
    BOOST_CHECK_THROW(read_json("{,", &parsed), ParseError);
    BOOST_CHECK_THROW(read_json("{5: 5}", &parsed), ParseError);
    BOOST_CHECK_THROW(read_json("{\"a\"", &parsed), ParseError);
    BOOST_CHECK_THROW(read_json("{\"a\":", &parsed), ParseError);
    BOOST_CHECK_THROW(read_json("{\"a\":5", &parsed), ParseError);
    BOOST_CHECK_THROW(read_json("{\"a\":5 ]", &parsed), ParseError);
}

BOOST_AUTO_TEST_CASE(obj)
{
    auto obj = read_json<MyType>("{\"x\": 0, \"y\": 10, \"z\": -5}");
    BOOST_CHECK_EQUAL(0, obj.x);
    BOOST_CHECK_EQUAL(10, obj.y);
    BOOST_CHECK_EQUAL(-5, obj.z);


    BOOST_CHECK_THROW(read_json<MyType>("{\"x\": 0, \"y\": true, \"z\": -5}"), ParseError);
    BOOST_CHECK_THROW(read_json<MyType>("5"), ParseError);
    BOOST_CHECK_THROW(read_json<MyType>("{}"), ParseError);
    BOOST_CHECK_THROW(read_json<MyType>("{\"x\": 0, \"y\": 10}"), ParseError);
    BOOST_CHECK_THROW(read_json<MyType>("{\"x\": 0, \"y\": 10, \"z\": -5, \"w\": 1}"), ParseError);
    BOOST_CHECK_THROW(read_json<MyType>("{\"x\": 0, \"y\": 10, \"z\": -5, \"x\": 15}"), ParseError);
    BOOST_CHECK_THROW(read_json<MyType>("{\"x\": 0, \"y\": 10, \"z\": -5, 7: 15}"), ParseError);


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

BOOST_AUTO_TEST_CASE(obj_complex)
{
    std::string json =
        "{"
        "   \"total\": 102,"
        "   \"first\": 100,"
        "   \"data\": ["
        "       {\"author\": \"Ben\", \"text\": \"Hi\"},"
        "       {\"author\": \"Tim\", \"text\": \"Hi Ben\", \"admin\": true}"
        "   ]"
        "}";
    auto comments = read_json<ResponsePage<Comment>>(json);

    BOOST_CHECK_EQUAL(100, comments.first);
    BOOST_CHECK_EQUAL(102, comments.total);
    BOOST_CHECK_EQUAL(2, comments.data.size());
    if (comments.data.size() == 2)
    {
        BOOST_CHECK_EQUAL("Ben", comments.data[0].author);
        BOOST_CHECK_EQUAL("Hi", comments.data[0].text);
        BOOST_CHECK_EQUAL("Tim", comments.data[1].author);
        BOOST_CHECK_EQUAL("Hi Ben", comments.data[1].text);
    }
}

BOOST_AUTO_TEST_SUITE_END()
