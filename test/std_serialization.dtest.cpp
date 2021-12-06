#include <dtest.h>
#include <std_serialization.h>
#include "test_serializers.cpp"
#include "test_serializable.cpp"

module("std_serialization")
.dependsOn({
    "stream-serializer",
    "random-access-serializer"
});

#define TEST_SIZE (1024)

unit("std_serialization", "vector<int>")
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

unit("std_serialization", "vector<Serializable>")
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

unit("std_serialization", "string")
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
