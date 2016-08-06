#pragma once
#include "Parser.hpp"
#include "Time.hpp"
#include <unordered_map>
#include <limits>
namespace json
{
    namespace detail
    {
        template<typename T>
        auto has_push_back_impl(int) -> decltype(
            std::declval<T>().push_back(std::declval<typename T::value_type>()),
            std::true_type{});
        template<typename T> std::false_type has_push_back_impl(...);
        template<typename T> struct has_push_back : public decltype(has_push_back_impl<T>(0)) {};

        template<typename T>
        auto has_kv_emplace_impl(int) -> decltype(
            std::declval<T>().emplace(std::string(), std::declval<typename T::value_type::second_type>()),
            std::true_type{});
        template<typename T> std::false_type has_kv_emplace_impl(...);
        template<typename T> struct has_kv_emplace : public decltype(has_kv_emplace_impl<T>(0)) {};
    }

    template<typename T> void read_json_int(Parser &parser, T *out)
    {
        *out = parser.next_int<T>();
    }
    template<typename T> void read_json_uint(Parser &parser, T *out)
    {
        *out = parser.next_uint<T>();
    }


    inline void read_json(Parser &parser, char *x)
    {
        read_json_int(parser, x);
    }
    inline void read_json(Parser &parser, short *x)
    {
        read_json_int(parser, x);
    }
    inline void read_json(Parser &parser, int *x)
    {
        read_json_int(parser, x);
    }
    inline void read_json(Parser &parser, long *x)
    {
        read_json_int(parser, x);
    }
    inline void read_json(Parser &parser, long long *x)
    {
        read_json_int(parser, x);
    }

    inline void read_json(Parser &parser, unsigned char *x)
    {
        read_json_uint(parser, x);
    }
    inline void read_json(Parser &parser, unsigned short *x)
    {
        read_json_uint(parser, x);
    }
    inline void read_json(Parser &parser, unsigned int *x)
    {
        read_json_uint(parser, x);
    }
    inline void read_json(Parser &parser, unsigned long *x)
    {
        read_json_uint(parser, x);
    }
    inline void read_json(Parser &parser, unsigned long long *x)
    {
        read_json_uint(parser, x);
    }

    inline void read_json(Parser &parser, float *x)
    {
        *x = (float)parser.next_double();
    }
    inline void read_json(Parser &parser, double *x)
    {
        *x = parser.next_double();
    }

    inline void read_json(Parser &parser, bool *x)
    {
        auto tok = parser.next();
        if (tok.type == Token::TRUE_VAL) *x = true;
        else if (tok.type == Token::FALSE_VAL) *x = false;
        else throw ParseError("Expected boolean");
    }

    inline void read_json(Parser &parser, std::string *str)
    {
        parser.next_str(str);
    }

    /**Read from JSON string into out. */
    template<typename T>
    void read_json(const std::string &json, T *out)
    {
        Parser parser(json.data(), json.data() + json.size());
        read_json(parser, out);
        auto end = parser.next();
        if (end.type != Token::END) throw ParseError("Expected end");
    }

    /**Read from JSON string into an instance of type T, and return it. */
    template<typename T>
    T read_json(const std::string &json)
    {
        T tmp;
        read_json(json, &tmp);
        return tmp;
    }

    /**Read from parser into an instance of type T, and return it. */
    template<typename T>
    T read_json(Parser &parser)
    {
        T tmp;
        read_json(parser, &tmp);
        return tmp;
    }

    /**Read into a container that has push_back.*/
    template<typename T> void read_json_array(Parser &parser, T *container)
    {
        auto tok = parser.next();
        if (tok.type != Token::ARR_START) throw ParseError("Expected array");
        if (parser.try_next_arr_end()) return;

        do
        {
            typename T::value_type value;
            read_json(parser, &value);
            container->push_back(std::move(value));

            tok = parser.next();
        }
        while (tok.type == Token::ELEMENT_SEP);
        if (tok.type != Token::ARR_END) throw ParseError("Expected array end");
    }

    /**Read into a container that has emplace_back(str, val)*/
    template<typename T> void read_json_map(Parser &parser, T *container)
    {
        auto tok = parser.next();
        if (tok.type != Token::OBJ_START) throw ParseError("Expected object");
        if (parser.try_next_obj_end()) return;

        do
        {
            auto key = read_json<std::string>(parser);
            tok = parser.next();
            if (tok.type != Token::KEY_SEP) throw ParseError("Expected ':' object key-value seperator");

            typename T::value_type::second_type value;
            read_json(parser, &value);
            container->emplace(std::move(key), std::move(value));

            tok = parser.next();
        } while (tok.type == Token::ELEMENT_SEP);
        if (tok.type != Token::OBJ_END) throw ParseError("Expected object end");
    }

    template<typename T, typename std::enable_if<detail::has_push_back<T>::value>::type* = nullptr>
    void read_json(Parser &parser, T *arr_container)
    {
        read_json_array(parser, arr_container);
    }
    template<typename T, typename std::enable_if<detail::has_kv_emplace<T>::value>::type* = nullptr>
    void read_json(Parser &parser, T *map_container)
    {
        read_json_map(parser, map_container);
    }
    /**Read a time_t from a string.
     * This is not a read_json overload because time_t is a typedef for one of the int types.
     */
    inline void read_json_time(Parser &parser, time_t *t)
    {
        auto str = read_json<std::string>(parser);
        *t = parse_iso_time(str);
    }

    /**Skip past the next value. Works for objects and arrays. */
    inline void skip_value(Parser &parser)
    {
        size_t depth = 0;
        do
        {
            auto tok = parser.next();
            //this is less than ideal as allows for some invalid JSON
            //but as not tracking if currently in an array or object, this switch
            //cant tell the legality of various tokens
            switch (tok.type)
            {
            case Token::ARR_START:
            case Token::OBJ_START:
                ++depth;
                break;
            case Token::ARR_END:
            case Token::OBJ_END:
                if (depth == 0) throw ParseError("Expected object or array end");
                --depth;
                break;
            case Token::ELEMENT_SEP:
            case Token::KEY_SEP:
                break;
            case Token::TRUE_VAL:
            case Token::FALSE_VAL:
            case Token::NULL_VAL:
            case Token::STRING:
            case Token::INTEGER:
            case Token::NUMBER:
                break;
            default: throw ParseError("Expected value");
            }
        }
        while (depth > 0);
    }


    namespace detail
    {
        template<typename T>
        struct Field
        {
            typedef void(*ReadField)(Parser &parser, T *obj);
            ReadField read;
            size_t index;
        };
        template<typename T>
        using Fields = std::unordered_map<std::string, Field<T>>;
    }

    struct ErrorUnknown
    {
        void operator()(Parser &parser, const std::string &key)
        {
            throw ParseError("Unknown key " + key);
        }
    };
    struct IgnoreUnknown
    {
        void operator()(Parser &parser, const std::string &key)
        {
            skip_value(parser);
        }
    };

    template<typename T, typename ErrorPolicy = ErrorUnknown, size_t N = 0>
    class ObjectFieldReader
    {
    public:
        typedef detail::Field<T> Field;
        typedef typename Field::ReadField ReadField;
        typedef detail::Fields<T> Fields;

        ObjectFieldReader() : fields()
        {
            assert(N == 0);
        }
        explicit ObjectFieldReader(detail::Fields<T> &&fields) : fields(std::move(fields))
        {
            assert(N == this->fields.size());
        }

        /**Add a field using a member variable pointer.
         * Due to current limitations in the C++ template syntax as of C++11,
         * both the type and the pointer itself must be specified.
         * 
         * e.g: @code
         * static auto const reader = ObjectFieldReader<MyType>().
         *      add<decltype(MyType::x), &MyType::x>("x").
         *      add<decltype(MyType::y), &MyType::y>("y").
         *      add<decltype(MyType::z), &MyType::z>("z");
         * @endcode
         */
        template<typename U, U T::*ptr>
        ObjectFieldReader<T, ErrorPolicy, N + 1> add(const std::string &name)
        {
            return add(name, do_read_field<U, ptr>);
        }

        /**Adds a property with a specified "void (Parser &parser, U *obj)" function
         * to convert the value.
         * e.g: @code
         * static auto const reader = ObjectFieldReader<MyType>().
         *      add<time_t, &MyType::time, read_json_time>("time");
         * @endcode
         */
        template<typename U, U T::*ptr, void(*read_func)(Parser &parser, U *out)>
        ObjectFieldReader<T, ErrorPolicy, N + 1> add(const std::string &name)
        {
            return add(name, do_read_field<U, ptr, read_func>);
        }
        /**Adds a property with a specified "void (Parser &parser, T *obj)" read function.*/
        ObjectFieldReader<T, ErrorPolicy, N + 1> add(const std::string &name, ReadField read)
        {
            auto index = fields.size();
            fields[name] = { read, index };
            return ObjectFieldReader<T, ErrorPolicy, N + 1>(std::move(fields));
        }

        /**Read an object with the specified fields into out.*/
        void read(Parser &parser, T *out)const
        {
            bool visited[N] = { 0 };
            size_t count = 0;
            auto tok = parser.next();
            if (tok.type != Token::OBJ_START) throw ParseError("Expected object start");
            if (parser.try_next_obj_end()) throw ParseError("Missing keys, unexpected empty object");
            std::string key;
            do
            {
                //key
                key.clear();
                parser.next_str(&key);
                //find field
                auto field = fields.find(key);
                //':' seperator
                tok = parser.next();
                if (tok.type != Token::KEY_SEP) throw ParseError("Expected ':'");
                //read field
                if (field != fields.end())
                {
                    if (visited[field->second.index]) throw ParseError("Duplicate key " + key);
                    field->second.read(parser, out);
                    visited[field->second.index] = true;
                    ++count;
                }
                else ErrorPolicy()(parser, key);
                
                //next field
                tok = parser.next(); // ':' or '}'
            }
            while (tok.type == Token::ELEMENT_SEP);
            if (tok.type != Token::OBJ_END) throw ParseError("Expected object end");
            if (count != N) throw ParseError("Missing keys");
        }
    private:
        template <typename U, U T::*ptr>
        static void do_read_field(Parser &parser, T *obj)
        {
            U *field = &(obj->*ptr);
            read_json(parser, field);
        }
        template <typename U, U T::*ptr, void(*read_func)(Parser &parser, U *out)>
        static void do_read_field(Parser &parser, T *obj)
        {
            U *field = &(obj->*ptr);
            read_func(parser, field);
        }

        Fields fields;
    };
}
