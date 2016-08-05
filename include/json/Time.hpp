#pragma once
#include <ctime>
#include <string>
#include <stdexcept>

namespace json
{
    namespace
    {
        int parse_time_num(char c, const std::string &time_str)
        {
            if (c >= '0' && c <= '9') return c - '0';
            else throw std::runtime_error("Invalid time " + time_str);
        }
        template<size_t len>
        int parse_time_num(const char *str, const std::string &time_str)
        {
            int sum = 0;
            for (size_t i = 0; i < len; ++i)
            {
                sum *= 10;
                sum += parse_time_num(str[i], time_str);
            }
            return sum;
        }
    }
    /**Parse time. Currently supports only UTC 'YYYY-MM-DDTHH:MM:SS' with optional trailing Z.*/
    inline time_t parse_iso_time(const std::string &str)
    {
        auto strp = str.c_str();
        auto len = str.size();
        if (len < sizeof("YYYY-MM-DDTHH:MM:SS") - 1) throw std::runtime_error("Invalid time " + str);
        if (len > sizeof("YYYY-MM-DDTHH:MM:SSZ") - 1) throw std::runtime_error("Invalid time " + str);
        //validate static components
        if (len == sizeof("YYYY-MM-DDTHH:MM:SSZ") - 1 && str[19] != 'Z') throw std::runtime_error("Invalid time " + str);
        if (strp[4] != '-' || strp[7] != '-' || strp[10] != 'T' || strp[13] != ':' || strp[16] != ':')
            throw std::runtime_error("Invalid time " + str);
        //parse numbers
        tm tm;

        tm.tm_year = parse_time_num<4>(strp + 0, str);
        tm.tm_mon = parse_time_num<2>(strp + 5, str);
        tm.tm_mday = parse_time_num<2>(strp + 8, str);

        tm.tm_hour = parse_time_num<2>(strp + 11, str);
        tm.tm_min = parse_time_num<2>(strp + 14, str);
        tm.tm_sec = parse_time_num<2>(strp + 17, str);
        //adjust for tm
        tm.tm_year -= 1900;
        tm.tm_mon -= 1;
        tm.tm_isdst = 0;
        //validate
        if (tm.tm_year < 0 || tm.tm_year > 1100 ||
            tm.tm_mon < 0 || tm.tm_mon > 11 ||
            tm.tm_mday < 1 || tm.tm_mday > 31 ||
            tm.tm_hour < 0 || tm.tm_hour > 23 ||
            tm.tm_min < 0 || tm.tm_min > 59 ||
            tm.tm_sec < 0 || tm.tm_sec > 60)
        {
            throw std::runtime_error("Invalid time " + str);
        }
        //convert to time_t
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
