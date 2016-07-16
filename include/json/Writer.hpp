#pragma once
#include <cstring>
#include <string>
#include <sstream>
namespace json
{
    /**@brief Writes JSON tokens to a string buffer.
     * 
     * Provides methods to write the basic JSON primitives,
     * and the key, and prop helpers for writing key-value's in objects.
     * 
     * The template method value<T> calls write_json. Users may provide overloads of write_json
     * for custom types, and ADL will be used to call them.
     */
    class Writer
    {
    public:
        Writer() : buf(), first_el(true) {}
        Writer(const Writer&) = delete;
        Writer& operator = (const Writer&) = delete;

        std::string& str() { return buf; }

        void start_arr()
        {
            check_first();
            put('[');
            first_el = true;
        }
        void end_arr()
        {
            put(']');
            first_el = false;
        }

        void start_obj()
        {
            check_first();
            put('{');
            first_el = true;
        }
        void end_obj()
        {
            put('}');
            first_el = false;
        }

        /**Write a null value.*/
        void null()
        {
            check_first();
            buf += "null";
        }
        /**Write directly to buffer. Assumes the provided string is a valid value, and needs
         * no processing.
         */
        void raw_value(const std::string &str)
        {
            check_first();
            buf += str;
        }
        void do_value(bool b)
        {
            check_first();
            buf += b ? "true" : "false";
        }
        void do_value(double x)
        {
            check_first();
            std::stringstream ss;
            ss << x;
            buf += ss.str();
        }
        void do_value(long long x)
        {
            check_first();
            buf += std::to_string(x);
        }
        void do_value(unsigned long long x)
        {
            check_first();
            buf += std::to_string(x);
        }
        void do_value(const char *str)
        {
            check_first();
            put('"');
            for (; *str; ++str) do_str_chr(*str);
            put('"');
        }

        /**Writes a string followed by a ':'.*/
        void key(const char *str)
        {
            do_value(str);
            put(':');
            first_el = true;
        }
        void key(const std::string &str)
        {
            key(str.c_str());
        }

        /**Writes an object property as a string, ':', then using write_json to write the value.*/
        template<class T> void prop(const char *str, T &&val)
        {
            key(str);
            value(std::forward<T>(val));
        }
        template<class T> void prop(const std::string &str, T &&val)
        {
            key(str);
            value(std::forward<T>(val));
        }

        /**Writes a value by forwarding to a write_json overload.*/
        template<typename T> void value(const T &v)
        {
            write_json(*this, v);
        }
    private:
        std::string buf;
        /**First array or object member.*/
        bool first_el;

        void check_first()
        {
            if (first_el) first_el = false;
            else put(',');
        }
        void put(char c) { buf += c; }
        void do_str_chr(char c)
        {
            switch (c)
            {
            case '\b': put('\\'); put('b'); break;
            case '\f': put('\\'); put('f'); break;
            case '\n': put('\\'); put('n'); break;
            case '\r': put('\\'); put('r'); break;
            case '\t': put('\\'); put('t'); break;
            case '\"': put('\\'); put('\"'); break;
            case '\\': put('\\'); put('\\'); break;
            default: put(c); break;
            }
        }
    };

    namespace detail
    {
        template<typename T>
        auto is_iterable_impl(int) -> decltype(
            begin(std::declval<T&>()) == end(std::declval<T&>()),
            *begin(std::declval<T&>()),
            ++begin(std::declval<T&>()),
            std::true_type{}
        );
        template<typename T>
        std::false_type is_iterable_impl(...);

        /**Consider an object iterable, if the following are valid:
         *
         *   - iterator = begin(T())
         *   - iterator = end(T())
         *   - iterator == iterator
         *   - *iterator
         *   - ++iterator
         *
         * The decltype will only be valid, resolving to std::true_type if the listed operations
         * are valid.
         */
        template <typename T> struct is_iterable : public decltype(is_iterable_impl<T>(0)) {};

        using std::to_string;
        template<typename T> auto has_to_string_impl(int) -> decltype(to_string(std::declval<T>()));
        template<typename T> std::false_type has_to_string_impl(...);
        /**to_string(T()) exists.*/
        template<typename T> struct has_to_string : public decltype(has_to_string_impl<T>(0)) {};

    }

    inline void write_json(Writer &writer, bool b) { writer.do_value(b); }

    inline void write_json(Writer &writer, float x) { writer.do_value(x); }
    inline void write_json(Writer &writer, double x) { writer.do_value(x); }

    inline void write_json(Writer &writer, char x) { writer.do_value((long long)x); }
    inline void write_json(Writer &writer, short x) { writer.do_value((long long)x); }
    inline void write_json(Writer &writer, int x) { writer.do_value((long long)x); }
    inline void write_json(Writer &writer, long x) { writer.do_value((long long)x); }
    inline void write_json(Writer &writer, long long x) { writer.do_value(x); }

    inline void write_json(Writer &writer, unsigned char x) { writer.do_value((unsigned long long)x); }
    inline void write_json(Writer &writer, unsigned short x) { writer.do_value((unsigned long long)x); }
    inline void write_json(Writer &writer, unsigned int x) { writer.do_value((unsigned long long)x); }
    inline void write_json(Writer &writer, unsigned long x) { writer.do_value((unsigned long long)x); }
    inline void write_json(Writer &writer, unsigned long long x) { writer.do_value(x); }

    inline void write_json(Writer &writer, const char *x) { writer.do_value(x); }
    inline void write_json(Writer &writer, const std::string &x) { writer.do_value(x.c_str()); }

    /**Generic template for arrays. Writes a JSON array, using write_json for each element.
     * 
     * iterable may be any value that can be used with the C++11 range-based for loop.
     */
    template<typename T> void write_json_array(Writer &writer,  const T &iterable)
    {
        writer.start_arr();
        for (auto &i : iterable)
        {
            writer.value(i);
        }
        writer.end_arr();
    }
    /**Template write_json for any array type using write_json_array.*/
    template<typename T, typename std::enable_if<detail::is_iterable<T>::value>::type * = nullptr>
    void write_json(Writer &writer, const T &iterable)
    {
        write_json_array(writer, iterable);
    }
    /**Template overload for pointers.*/
    template<typename T> void write_json(Writer &writer, const T *val)
    {
        if (val) write_json(writer, *val);
        else writer.null();
    }
    template<typename T> void write_json(Writer &writer, T *val)
    {
        write_json<T>(writer, (const T*)val);
    }
    /**Basic template JSON writer. This default template converts the value to a string
      * via to_string, then writes that as a JSON string.
    */
    template<typename T, typename std::enable_if<detail::has_to_string<T>::value>::type * = nullptr>
    void write_json(Writer &writer, const T &val)
    {
        using std::to_string;
        writer.do_value(to_string(val));
    }

    /**Converts some object to a JSON string, by creating a Writer then calling Writer::value on
     * it and returning the string buffer.
     */
    template<typename T> std::string to_json(const T &obj)
    {
        Writer writer;
        writer.value(obj);
        return std::move(writer.str());
    }
}
