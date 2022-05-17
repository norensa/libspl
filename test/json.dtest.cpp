/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <json.h>

module("json")
.dependsOn({
    "exception",
    "string-conversions",
    "list",
    "hash-map"
});

using namespace spl;

unit("json", "encode-uint32")
.body([] {
    assert(JSON::encode((uint32) 123) == "123");
});

unit("json", "decode-uint32")
.body([] {
    assert(JSON::decode<uint32>("123") == (uint32) 123);

    try {
        JSON::decode<uint32>("");
        fail("Empty string decoded");
    }
    catch (const JSONDecodeError &e) { }

    try {
        JSON::decode<uint32>("12,");
        fail("Decode succeeded despite extra characters");
    }
    catch (const JSONDecodeError &e) { }
});

unit("json", "encode-bool")
.body([] {
    assert(JSON::encode(true) == "true");
    assert(JSON::encode(false) == "false");
});

unit("json", "decode-bool")
.body([] {
    assert(JSON::decode<bool>("true") == true);
    assert(JSON::decode<bool>("false") == false);

    try {
        JSON::decode<bool>("");
        fail("Empty string decoded");
    }
    catch (const JSONDecodeError &e) { }

    try {
        JSON::decode<bool>("true,");
        fail("Decode succeeded despite extra characters");
    }
    catch (const JSONDecodeError &e) { }
});

unit("json", "encode-str")
.body([] {
    assert(JSON::encode("hello world") == "\"hello world\"");

    assert(JSON::encode("\"hi\"") == "\"\\\"hi\\\"\"");

    assert(JSON::encode("\b\f\n\r\t\"\\") == "\"\\b\\f\\n\\r\\t\\\"\\\\\"");
});

unit("json", "decode-str")
.body([] {
    assert(JSON::decode<std::string>("\"hello world\"") == "hello world");

    assert(JSON::decode<std::string>("\"\\\"hi\\\"\"") == "\"hi\"");

    assert(JSON::decode<std::string>("\"\\b\\f\\n\\r\\t\\\"\\\\\"") == "\b\f\n\r\t\"\\");

    try {
        JSON::decode<std::string>("");
        fail("Empty string decoded");
    }
    catch (const JSONDecodeError &e) { }

    try {
        JSON::decode<std::string>("hello world");
        fail("String without double quotes decoded");
    }
    catch (const JSONDecodeError &e) { }
});

unit("json", "encode-list")
.body([] {
    assert(JSON::encode(List<int>()) == "[ ]");

    assert(JSON::encode(List<int>({ 1 })) == 
        "[\n"
        "  1\n"
        "]"
    );

    assert(JSON::encode(List<int>({ 1, 2, 3 })) == 
        "[\n"
        "  1,\n"
        "  2,\n"
        "  3\n"
        "]"
    );

    assert(JSON::encode(List<std::string>({ "hi", "hey", "hello" })) ==
        "[\n"
        "  \"hi\",\n"
        "  \"hey\",\n"
        "  \"hello\"\n"
        "]"
    );

    assert(JSON::encode(List<HashMap<std::string, int>>({
        {
            { "prop1", 1 },
            { "prop2", 2 },
        },
        {

        },
        {
            { "prop2", 2 },
        }
    })) == 
        "[\n"
        "  {\n"
        "    \"prop1\": 1,\n"
        "    \"prop2\": 2\n"
        "  },\n"
        "  { },\n"
        "  {\n"
        "    \"prop2\": 2\n"
        "  }\n"
        "]"
    );
});

unit("json", "decode-list")
.body([] {
    assert(JSON::decode<List<int>>("[]").empty());

    assert(JSON::decode<List<int>>("[ ]").empty());

    assert(JSON::decode<List<int>>("[  \n   ]").empty());

    {
        auto l = JSON::decode<List<int>>("[ 1 ]");
        assert(l.nonEmpty());
        assert(l.size() == 1);
        auto it = l.begin();
        assert(*it == 1);
    }

    {
        auto l = JSON::decode<List<int>>("[ 1, 2, 3 ]");
        assert(l.nonEmpty());
        assert(l.size() == 3);
        auto it = l.begin();
        assert(*it++ == 1);
        assert(*it++ == 2);
        assert(*it++ == 3);
    }

    {
        auto l = JSON::decode<List<std::string>>("[ \"hi\", \"hey\" \n]");
        assert(l.nonEmpty());
        assert(l.size() == 2);
        auto it = l.begin();
        assert(*it++ == "hi");
        assert(*it++ == "hey");
    }

    {
        auto l = JSON::decode<List<HashMap<std::string, int>>>("[ { }, { \"prop1\": 1 } ]");
        assert(l.nonEmpty());
        assert(l.size() == 2);

        auto it = l.begin();
        assert(it->empty());

        ++it;
        assert(it->nonEmpty());
        assert(it->size() == 1);
        assert(it->contains("prop1"));
        assert(it->get("prop1") == 1);
    }

    try {
        JSON::decode<List<int>>("[ 1, ]");
        fail("trailing comma");
    }
    catch (const JSONDecodeError &) { }
});

unit("json", "encode-map")
.body([] {
    assert(JSON::encode(HashMap<std::string, int>()) == "{ }");

    assert(JSON::encode(HashMap<std::string, int>({
        { "prop1", 1 }
    })) == "{\n  \"prop1\": 1\n}");

    assert(JSON::encode(HashMap<std::string, int>({
        { "prop1", 1 },
        { "prop2", 2 }
    })) == 
        "{\n"
        "  \"prop1\": 1,\n"
        "  \"prop2\": 2\n"
        "}"
    );

    assert(JSON::encode(HashMap<std::string, List<int>>({
        { "prop1", { 1 } },
        { "prop2", { 1, 2 } }
    })) == 
        "{\n"
        "  \"prop1\": [\n"
        "    1\n"
        "  ],\n"
        "  \"prop2\": [\n"
        "    1,\n"
        "    2\n"
        "  ]\n"
        "}"
    );
});

unit("json", "decode-map")
.body([] {
    assert(( JSON::decode<HashMap<std::string, int>>("{}").empty() ));

    assert(( JSON::decode<HashMap<std::string, int>>("{ }").empty() ));

    assert(( JSON::decode<HashMap<std::string, int>>("{ \n}").empty() ));

    {
        auto m = JSON::decode<HashMap<std::string, int>>("{ \"prop1\": 1 }");
        assert(m.nonEmpty());
        assert(m.size() == 1);
        assert(m.contains("prop1"));
        assert(m.get("prop1") == 1);
    }

    {
        auto m = JSON::decode<HashMap<std::string, int>>("{ \"prop1\": 1, \"prop2\" : 2 }");
        assert(m.nonEmpty());
        assert(m.size() == 2);
        assert(m.contains("prop1"));
        assert(m.get("prop1") == 1);
        assert(m.contains("prop2"));
        assert(m.get("prop2") == 2);
    }

    {
        auto m = JSON::decode<HashMap<std::string, List<int>>>("{ \"prop1\": [], \"prop2\" : [ 2 ],\n \"prop3\": \n [ 1, 2 ] }");
        assert(m.nonEmpty());
        assert(m.size() == 3);

        assert(m.contains("prop1"));
        assert(m.get("prop1").empty());

        assert(m.contains("prop2"));
        auto l = m.get("prop2");
        assert(l.size() == 1);
        assert(*l.begin() == 2);

        assert(m.contains("prop3"));
        l = m.get("prop3");
        assert(l.size() == 2);
        auto it = l.begin();
        assert(*it++ == 1);
        assert(*it++ == 2);
    }

    try {
        JSON::decode<HashMap<std::string, int>>("{ \"prop1\": 1 , }");
        fail("trailing comma");
    }
    catch (const JSONDecodeError &) { }
});
