#include <dtest.h>
#include "test_serializers.cpp"
#include "test_serializable.cpp"

module("container-serialization")
.dependsOn({
    "stream-serializer",
    "random-access-serializer"
});

using namespace spl;

#define TEST_SIZE (1024)

// list ////////////////////////

#include <list.h>

unit("container-serialization", "list<int>")
.dependsOn("list")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.insert(dtest_random() * TEST_SIZE);
    }

    MemoryOutputStreamSerializer out;
    out << l;
    out.flush();

    List<int> l2;
    auto in = out.toInput();
    in >> l2;

    assert(l.size() == l2.size());

    for (auto it1 = l.begin(), it2 = l2.begin(); it1 != l.end(); ++it1, ++it2) {
        assert(*it1 == *it2);
    }
});

unit("container-serialization", "list<serializable>")
.dependsOn("list")
.body([] {
    auto l = List<StreamSerializable>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.insert(StreamSerializable());
    }

    MemoryOutputStreamSerializer out;
    out << l;
    out.flush();

    for (auto &x : l) {
        assert(x.serialized());
    }

    List<StreamSerializable> l2;
    auto in = out.toInput();
    in >> l2;

    assert(l.size() == l2.size());

    for (auto &x : l2) {
        assert(x.deserialized());
    }
});

unit("container-serialization", "list<non-serializable>")
.dependsOn("list")
.expect(Status::FAIL)
.body([] {
    auto l = List<StreamSerializable_NotCopyAssignable>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.insert(StreamSerializable_NotCopyAssignable());
    }

    MemoryOutputStreamSerializer out;
    out << l;
    err("Object should not be serialized");
});

unit("container-serialization", "list<non-deserializable>")
.dependsOn("list")
.expect(Status::FAIL)
.body([] {
    auto l = List<StreamSerializable_NotConstructible>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.insert(StreamSerializable_NotConstructible(0));
    }

    MemoryOutputStreamSerializer out;
    out << l;
    out.flush();

    List<StreamSerializable_NotConstructible> l2;
    auto in = out.toInput();
    in >> l2;
    err("Object should not be deserialized");
});

unit("container-serialization", "parallel::list<int>")
.dependsOn("parallel::list")
.body([] {
    auto l = parallel::List<int>();

    #pragma omp parallel for
    for (int i = 0; i < TEST_SIZE; ++i) {
        l.insert(dtest_random() * TEST_SIZE);
    }

    MemoryOutputStreamSerializer out;
    out << l;
    out.flush();

    parallel::List<int> l2;
    auto in = out.toInput();
    in >> l2;

    assert(l.size() == l2.size());

    for (auto it1 = l.begin(), it2 = l2.begin(); it1 != l.end(); ++it1, ++it2) {
        assert(*it1 == *it2);
    }
});

unit("container-serialization", "parallel::list<serializable>")
.dependsOn("parallel::list")
.body([] {
    auto l = parallel::List<StreamSerializable>();

    #pragma omp parallel for
    for (int i = 0; i < TEST_SIZE; ++i) {
        l.insert(StreamSerializable());
    }

    MemoryOutputStreamSerializer out;
    out << l;
    out.flush();

    for (auto &x : l) {
        assert(x.serialized());
    }

    parallel::List<StreamSerializable> l2;
    auto in = out.toInput();
    in >> l2;

    assert(l.size() == l2.size());

    for (auto &x : l2) {
        assert(x.deserialized());
    }
});
