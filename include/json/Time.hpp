#pragma once
#include <ctime>
#include <string>
#include <sstream>
#include <stdexcept>

namespace json
{
    /**Parse time. Currently supports only UTC 'YYYY-MM-DDTHH:MM:SS' with optional trailing Z.*/
    inline time_t parse_iso_time(const std::string &str)
    {
        tm tm;
        char sep;

        std::stringstream ss(str);
        ss >> tm.tm_year >> sep >> tm.tm_mon >> sep >> tm.tm_mday;
        ss >> sep;
        ss >> tm.tm_hour >> sep >> tm.tm_min >> sep >> tm.tm_sec;

        if (ss.fail()) throw std::runtime_error("Invalid time " + str);
        ss >> sep;
        if (ss.good())
        {
            if (sep != 'Z') throw std::runtime_error("Invalid time " + str);
        }
        ss >> sep;
        if (!ss.eof()) throw std::runtime_error("Invalid time " + str);

        tm.tm_year -= 1900;
        tm.tm_mon -= 1;
        tm.tm_isdst = 0;

        if (tm.tm_year < 0 || tm.tm_year > 1100 ||
            tm.tm_mon < 0 || tm.tm_mon > 11 ||
            tm.tm_mday < 1 || tm.tm_mday > 31 ||
            tm.tm_hour < 0 || tm.tm_hour > 23 ||
            tm.tm_min < 0 || tm.tm_min > 59 ||
            tm.tm_sec < 0 || tm.tm_sec > 60)
        {
            throw std::runtime_error("Invalid time " + str);
        }

#ifdef _MSC_VER
        return _mkgmtime(&tm);
#else
        return timegm(&tm);
#endif
    }
    /**Serialise UTC time to string in 'YYYY-MM-DDTHH:MM:SSZ' format.*/
    inline std::string time_to_iso_str(time_t t)
    {
        tm tm;
#ifdef _MSC_VER
        gmtime_s(&tm, &t);
#else
        gmtime_r(&t, &tm);
#endif
        char buffer[sizeof("YYYY-MM-DDTHH:MM:SSZ")];
        auto len = strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm);
        if (!len) throw std::runtime_error("stftime for json::time_to_iso_str failed");

        return{ buffer, len };
    }
}
