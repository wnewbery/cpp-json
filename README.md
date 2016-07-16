# cpp-json
C++ JSON library with pull API


JSON parser built around a pull parser that yields single tokens, allowing for simple reading of complex structures without an intermediate DOM, and without the complexity of SAX parser callbacks.

This is similar to the Java `javax.json.stream.JsonParser` http://docs.oracle.com/javaee/7/api/javax/json/stream/JsonParser.html

# Reading JSON

The design also makes use of C++ templating techniques to maintain simple reader and writer API's.

For example this JSON:

    {
      "a": true,
      "b": [1, 2, { x: -5, y: 10 } ]
    }

Can simply be consumed piece by piece:

    Parser parser(json_str);
    parser.next(); //OBJ_START
    parser.next(); //STRING, "a"
    parser.next(); //KEY_SEP //':', so expect value next
    parser.next(); //TRUE_VAL
    parser.next(); //ELEMENT_SEP //',', so expect another key-value pair
    parser.next(); //STRING, "b"
    parser.next(); //KEY_SEP
    parser.next(); //ARR_START
    parser.next(); //NUMBER, 1
    parser.next(); //ELEMENT_SEP
    parser.next(); //NUMBER, 2
    parser.next(); //OBJ_START
    parser.next(); //STRING, "x"
    parser.next(); //KEY_SEP
    parser.next(); //NUMBER, -5
    parser.next(); //ELEMENT_SEP
    parser.next(); //STRING, "y"
    parser.next(); //KEY_SEP
    parser.next(); //NUMBER, 10
    parser.next(); //OBJ_END
    parser.next(); //ARR_END
    parser.next(); //OBJ_END
    parser.next(); //END

A more complex example, including a reusable template component.

    template<typename T>
    struct ResponsePage
    {
        size_t first;
        size_t total;
        std::vector<T> data; //fields with a push_back have a default implementation
    };
    template<typename T> void read_json(Parser &parser, ResponsePage<T> *val)
    {
        static const auto reader = json::ObjectFieldReader<ResponsePage<T>>().
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
        static const auto reader = json::ObjectFieldReader<Comment, IgnoreUnknown>().
            add<decltype(Comment::author), &Comment::author>("author").
            add<decltype(Comment::text), &Comment::text>("text");
        reader.read(parser, val);
    }
    
    
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

# Writing JSON
Converting objects to a JSON string is done by the `json::Writer` object, with the help with `void write_json(json::Writer&, const T &value)` overloads.

And since dont need to worry about object key order, unknown keys or duplicates for output, these overloads are far simpler than the `read_json` ones.

    template<typename T> void write_json(Writer &writer, const ResponsePage<T> &page)
    {
        writer.start_obj();
        writer.prop("first", page.first);
        writer.prop("total", page.total);
        writer.prop("data", page.data);
        writer.end_obj();
    }
    void write_json(Writer &writer, const Comment &comment)
    {
        writer.start_obj();
        writer.prop("author", comment.author);
        writer.prop("text", comment.text);
        writer.end_obj();
    }

    ResponsePage<Comment> page = { 100, 102, {
        { "Ben", "Hi" },
        { "Tim", "Hi Ben"}
    }};
    std::string json_str = to_json(page);

