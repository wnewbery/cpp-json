#include <boost/test/unit_test.hpp>
#include "Time.hpp"

using namespace json;

BOOST_AUTO_TEST_SUITE(TestTime)

BOOST_AUTO_TEST_CASE(to_string)
{
    BOOST_CHECK_EQUAL("2016-06-24T09:47:55Z", time_to_iso_str(1466761675));
}

BOOST_AUTO_TEST_CASE(parse)
{
    BOOST_CHECK_EQUAL(1466761675, parse_iso_time("2016-06-24T09:47:55Z"));

    BOOST_CHECK_THROW(parse_iso_time(""), std::runtime_error);
    BOOST_CHECK_THROW(parse_iso_time("2016-06-24"), std::runtime_error);
    BOOST_CHECK_THROW(parse_iso_time("09:47:55"), std::runtime_error);

    BOOST_CHECK_THROW(parse_iso_time("1000-06-24T09:47:55Z"), std::runtime_error);
    BOOST_CHECK_THROW(parse_iso_time("4000-06-24T09:47:55Z"), std::runtime_error);
    BOOST_CHECK_THROW(parse_iso_time("2016-13-24T09:47:55Z"), std::runtime_error);
    BOOST_CHECK_THROW(parse_iso_time("2016-00-24T09:47:55Z"), std::runtime_error);
    BOOST_CHECK_THROW(parse_iso_time("2016-06-40T09:47:55Z"), std::runtime_error);
    BOOST_CHECK_THROW(parse_iso_time("2016-06-00T09:47:55Z"), std::runtime_error);

    BOOST_CHECK_THROW(parse_iso_time("2016-06-24T-1:47:55Z"), std::runtime_error);
    BOOST_CHECK_THROW(parse_iso_time("2016-06-24T24:47:55Z"), std::runtime_error);
    BOOST_CHECK_THROW(parse_iso_time("2016-06-24T09:60:55Z"), std::runtime_error);
    BOOST_CHECK_THROW(parse_iso_time("2016-06-24T09:-1:55Z"), std::runtime_error);
    BOOST_CHECK_THROW(parse_iso_time("2016-06-24T09:47:61Z"), std::runtime_error);
    BOOST_CHECK_THROW(parse_iso_time("2016-06-24T09:47:-1Z"), std::runtime_error);

    BOOST_CHECK_THROW(parse_iso_time("2016-06-24T09:47:55+01"), std::runtime_error);
    BOOST_CHECK_THROW(parse_iso_time("2016-06-24T09:47:55+01:30"), std::runtime_error);
    BOOST_CHECK_THROW(parse_iso_time("2016-06-24T09:47:55+0130"), std::runtime_error);
    BOOST_CHECK_THROW(parse_iso_time("2016-06-24T09:47:55-01"), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()
