/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <std_serialization.h>
#include "test_serializers.cpp"
#include "test_serializable.cpp"

module("std-serialization")
.dependsOn({
    "stream-serializer",
    "random-access-serializer"
});

#define TEST_SIZE (1024)

// std::vector /////////////////////////////////////////////////////////////////

unit("std-serialization", "vector<int>")
.body([] {
    std::vector<int> v(TEST_SIZE);
    for (auto i = 0; i < TEST_SIZE; ++i) {
        v[i] = dtest_random() * TEST_SIZE;
    }
    MemoryOutputStreamSerializer out;
    out << v;
    out.flush();

    std::vector<int> v2;
    auto in = out.toInput();
    in >> v2;

    assert(v.size() == v2.size());

    for (auto i = 0; i < TEST_SIZE; ++i) {
        assert(v[i] == v2[i]);
    }
});

unit("std-serialization", "vector<Serializable>")
.body([] {
    std::vector<StreamSerializable> v(TEST_SIZE);
    MemoryOutputStreamSerializer out;
    out << v;
    out.flush();

    for (auto &x : v) {
        assert(x.serialized());
    }

    std::vector<StreamSerializable> v2;
    auto in = out.toInput();
    in >> v2;

    assert(v.size() == v2.size());

    for (auto &x : v2) {
        assert(x.deserialized());
    }
});

unit("std-serialization", "vector<Serializable *>")
.body([] {
    std::vector<StreamSerializable *> v(TEST_SIZE);
    for (auto i = 0; i < TEST_SIZE; ++i) v[i] = new StreamSerializable();
    MemoryOutputStreamSerializer out;
    out << v;
    out.flush();

    for (auto x : v) {
        assert(x->serialized());
        delete x;
    }

    std::vector<StreamSerializable *> v2;
    auto in = out.toInput();
    in >> v2;

    assert(v.size() == v2.size());

    for (auto x : v2) {
        assert(x->deserialized());
        delete x;
    }
});

// std::string /////////////////////////////////////////////////////////////////

unit("std-serialization", "string")
.body([] {
    std::string s1 = "hello world!";
    MemoryOutputStreamSerializer out;
    out << s1;
    out.flush();

    std::string s2;
    auto in = out.toInput();
    in >> s2;

    assert(s1.size() == s2.size());
    assert(s1 == s2);
});

// std::pair ////////////////////////////////////////////////////////////////////

unit("std-serialization", "pair<int,int>")
.body([] {
    std::pair<int, int> p{ 3, 4 };
    MemoryOutputStreamSerializer out;
    out << p;
    out.flush();

    std::pair<int, int> p2;
    auto in = out.toInput();
    in >> p2;

    assert(p.first == p2.first);
    assert(p.second == p2.second);
});

unit("std-serialization", "pair<int,Serializable>")
.body([] {
    std::pair<int, StreamSerializable> p{ 3, StreamSerializable() };
    MemoryOutputStreamSerializer out;
    out << p;
    out.flush();

    assert(p.second.serialized());

    std::pair<int, StreamSerializable> p2;
    auto in = out.toInput();
    in >> p2;

    assert(p.first == p2.first);
    assert(p2.second.deserialized());
});

// std::map ////////////////////////////////////////////////////////////////////

unit("std-serialization", "map<int,int>")
.body([] {
    std::map<int, int> m;
    for (auto i = 0; i < TEST_SIZE; ++i) {
        m[i] = dtest_random() * TEST_SIZE;
    }
    MemoryOutputStreamSerializer out;
    out << m;
    out.flush();

    std::map<int, int> m2;
    auto in = out.toInput();
    in >> m2;

    assert(m.size() == m2.size());

    for (auto i = 0; i < TEST_SIZE; ++i) {
        assert(m[i] == m2[i]);
    }
});

unit("std-serialization", "map<int,Serializable>")
.body([] {
    std::map<int, StreamSerializable> m;
    for (auto i = 0; i < TEST_SIZE; ++i) {
        m[i] = StreamSerializable();
    }
    MemoryOutputStreamSerializer out;
    out << m;
    out.flush();

    for (auto &x : m) {
        assert(x.second.serialized());
    }

    std::map<int, StreamSerializable> m2;
    auto in = out.toInput();
    in >> m2;

    assert(m.size() == m2.size());

    for (auto i = 0; i < TEST_SIZE; ++i) {
        assert(m2[i].deserialized());
    }
});
