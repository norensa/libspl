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

// deque ///////////////////////

#include <deque.h>

unit("container-serialization", "deque<int>")
.dependsOn("deque")
.body([] {
    auto l = Deque<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.enqueue(dtest_random() * TEST_SIZE);
    }

    MemoryOutputStreamSerializer out;
    out << l;
    out.flush();

    Deque<int> l2;
    auto in = out.toInput();
    in >> l2;

    assert(l.size() == l2.size());

    for (auto it1 = l.begin(), it2 = l2.begin(); it1 != l.end(); ++it1, ++it2) {
        assert(*it1 == *it2);
    }
});

unit("container-serialization", "deque<serializable>")
.dependsOn("deque")
.body([] {
    auto l = Deque<StreamSerializable>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.enqueue(StreamSerializable());
    }

    MemoryOutputStreamSerializer out;
    out << l;
    out.flush();

    for (auto &x : l) {
        assert(x.serialized());
    }

    Deque<StreamSerializable> l2;
    auto in = out.toInput();
    in >> l2;

    assert(l.size() == l2.size());

    for (auto &x : l2) {
        assert(x.deserialized());
    }
});

unit("container-serialization", "deque<non-serializable>")
.dependsOn("deque")
.expect(Status::FAIL)
.body([] {
    auto l = Deque<StreamSerializable_NotCopyAssignable>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.enqueue(StreamSerializable_NotCopyAssignable());
    }

    MemoryOutputStreamSerializer out;
    out << l;
    err("Object should not be serialized");
});

unit("container-serialization", "deque<non-deserializable>")
.dependsOn("deque")
.expect(Status::FAIL)
.body([] {
    auto l = Deque<StreamSerializable_NotConstructible>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.enqueue(StreamSerializable_NotConstructible(0));
    }

    MemoryOutputStreamSerializer out;
    out << l;
    out.flush();

    Deque<StreamSerializable_NotConstructible> l2;
    auto in = out.toInput();
    in >> l2;
    err("Object should not be deserialized");
});

unit("container-serialization", "parallel::deque<int>")
.dependsOn("parallel::deque")
.body([] {
    auto l = parallel::Deque<int>();

    #pragma omp parallel for
    for (int i = 0; i < TEST_SIZE; ++i) {
        l.enqueue(dtest_random() * TEST_SIZE);
    }

    MemoryOutputStreamSerializer out;
    out << l;
    out.flush();

    parallel::Deque<int> l2;
    auto in = out.toInput();
    in >> l2;

    assert(l.size() == l2.size());

    for (auto it1 = l.begin(), it2 = l2.begin(); it1 != l.end(); ++it1, ++it2) {
        assert(*it1 == *it2);
    }
});

unit("container-serialization", "parallel::deque<serializable>")
.dependsOn("parallel::deque")
.body([] {
    auto l = parallel::Deque<StreamSerializable>();

    #pragma omp parallel for
    for (int i = 0; i < TEST_SIZE; ++i) {
        l.enqueue(StreamSerializable());
    }

    MemoryOutputStreamSerializer out;
    out << l;
    out.flush();

    for (auto &x : l) {
        assert(x.serialized());
    }

    parallel::Deque<StreamSerializable> l2;
    auto in = out.toInput();
    in >> l2;

    assert(l.size() == l2.size());

    for (auto &x : l2) {
        assert(x.deserialized());
    }
});

// hash-map ////////////////////

#include <hash_map.h>
#include "test_hashable.cpp"

unit("container-serialization", "hashmap<int,int>")
.dependsOn("hash-map")
.body([] {
    auto m = HashMap<int, int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, dtest_random() * TEST_SIZE);
    }

    MemoryOutputStreamSerializer out;
    out << m;
    out.flush();

    HashMap<int, int> m2;
    auto in = out.toInput();
    in >> m2;

    assert(m.size() == m2.size());

    for (auto &n : m) {
        assert(m2.contains(n.k));
        assert(m2.get(n.k) == n.v);
    }
});

unit("container-serialization", "hashmap<int,serializable>")
.dependsOn("hash-map")
.body([] {
    auto m = HashMap<int, StreamSerializable>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, StreamSerializable());
    }

    MemoryOutputStreamSerializer out;
    out << m;
    out.flush();

    for (auto &x : m) {
        assert(x.v.serialized());
    }

    HashMap<int, StreamSerializable> m2;
    auto in = out.toInput();
    in >> m2;

    assert(m.size() == m2.size());

    for (auto &n : m) {
        assert(m2.contains(n.k));
        assert(m2.get(n.k).deserialized());
    }
});

unit("container-serialization", "hashmap<hashableserializable,serializable>")
.dependsOn("hash-map")
.body([] {
    auto m = HashMap<HashableSerializableObj, StreamSerializable>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, StreamSerializable());
    }

    MemoryOutputStreamSerializer out;
    out << m;
    out.flush();

    for (auto &x : m) {
        assert(x.k.serialized);
        assert(x.v.serialized());
    }

    HashMap<HashableSerializableObj, StreamSerializable> m2;
    auto in = out.toInput();
    in >> m2;

    assert(m.size() == m2.size());

    for (auto &n : m) {
        assert(m2.contains(n.k));
        assert(m2.get(n.k).deserialized());
    }
});

unit("container-serialization", "parallel::hashmap<int,int>")
.dependsOn("parallel::hash-map")
.body([] {
    auto m = parallel::HashMap<int, int>();

    #pragma omp parallel for
    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, dtest_random() * TEST_SIZE);
    }

    MemoryOutputStreamSerializer out;
    out << m;
    out.flush();

    parallel::HashMap<int, int> m2;
    auto in = out.toInput();
    in >> m2;

    assert(m.size() == m2.size());

    for (auto &n : m) {
        assert(m2.contains(n.k));
        assert(m2.get(n.k) == n.v);
    }
});

unit("container-serialization", "parallel::hashmap<int,serializable>")
.dependsOn("parallel::hash-map")
.body([] {
    auto m = parallel::HashMap<int, StreamSerializable>();

    #pragma omp parallel for
    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, StreamSerializable());
    }

    MemoryOutputStreamSerializer out;
    out << m;
    out.flush();

    for (auto &x : m) {
        assert(x.v.serialized());
    }

    parallel::HashMap<int, StreamSerializable> m2;
    auto in = out.toInput();
    in >> m2;

    assert(m.size() == m2.size());

    for (auto &n : m) {
        assert(m2.contains(n.k));
        assert(m2.get(n.k).deserialized());
    }
});

unit("container-serialization", "parallel::hashmap<hashableserializable,serializable>")
.dependsOn("parallel::hash-map")
.body([] {
    auto m = parallel::HashMap<HashableSerializableObj, StreamSerializable>();

    #pragma omp parallel for
    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, StreamSerializable());
    }

    MemoryOutputStreamSerializer out;
    out << m;
    out.flush();

    for (auto &x : m) {
        assert(x.k.serialized);
        assert(x.v.serialized());
    }

    parallel::HashMap<HashableSerializableObj, StreamSerializable> m2;
    auto in = out.toInput();
    in >> m2;

    assert(m.size() == m2.size());

    for (auto &n : m) {
        assert(m2.contains(n.k));
        assert(m2.get(n.k).deserialized());
    }
});

unit("container-serialization", "hashmultimap<int,int>")
.dependsOn("hash-multimap")
.body([] {
    auto m = HashMultiMap<int, int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, dtest_random() * TEST_SIZE);
    }

    MemoryOutputStreamSerializer out;
    out << m;
    out.flush();

    HashMultiMap<int, int> m2;
    auto in = out.toInput();
    in >> m2;

    assert(m.size() == m2.size());

    for (auto &n : m) {
        assert(m2.contains(n.k));
        assert(m2.get(n.k) == n.v);
    }
});

unit("container-serialization", "hashmultimap<int,serializable>")
.dependsOn("hash-multimap")
.body([] {
    auto m = HashMultiMap<int, StreamSerializable>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, StreamSerializable());
    }

    MemoryOutputStreamSerializer out;
    out << m;
    out.flush();

    for (auto &x : m) {
        assert(x.v.serialized());
    }

    HashMultiMap<int, StreamSerializable> m2;
    auto in = out.toInput();
    in >> m2;

    assert(m.size() == m2.size());

    for (auto &n : m) {
        assert(m2.contains(n.k));
        assert(m2.get(n.k).deserialized());
    }
});

unit("container-serialization", "hashmultimap<hashableserializable,serializable>")
.dependsOn("hash-multimap")
.body([] {
    auto m = HashMultiMap<HashableSerializableObj, StreamSerializable>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, StreamSerializable());
    }

    MemoryOutputStreamSerializer out;
    out << m;
    out.flush();

    for (auto &x : m) {
        assert(x.k.serialized);
        assert(x.v.serialized());
    }

    HashMultiMap<HashableSerializableObj, StreamSerializable> m2;
    auto in = out.toInput();
    in >> m2;

    assert(m.size() == m2.size());

    for (auto &n : m) {
        assert(m2.contains(n.k));
        assert(m2.get(n.k).deserialized());
    }
});

unit("container-serialization", "parallel::hashmultimap<int,int>")
.dependsOn("parallel::hash-multimap")
.body([] {
    auto m = parallel::HashMultiMap<int, int>();

    #pragma omp parallel for
    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, dtest_random() * TEST_SIZE);
    }

    MemoryOutputStreamSerializer out;
    out << m;
    out.flush();

    parallel::HashMultiMap<int, int> m2;
    auto in = out.toInput();
    in >> m2;

    assert(m.size() == m2.size());

    for (auto &n : m) {
        assert(m2.contains(n.k));
        assert(m2.get(n.k) == n.v);
    }
});

unit("container-serialization", "parallel::hashmultimap<int,serializable>")
.dependsOn("parallel::hash-multimap")
.body([] {
    auto m = parallel::HashMultiMap<int, StreamSerializable>();

    #pragma omp parallel for
    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, StreamSerializable());
    }

    MemoryOutputStreamSerializer out;
    out << m;
    out.flush();

    for (auto &x : m) {
        assert(x.v.serialized());
    }

    parallel::HashMultiMap<int, StreamSerializable> m2;
    auto in = out.toInput();
    in >> m2;

    assert(m.size() == m2.size());

    for (auto &n : m) {
        assert(m2.contains(n.k));
        assert(m2.get(n.k).deserialized());
    }
});

unit("container-serialization", "parallel::hashmultimap<hashableserializable,serializable>")
.dependsOn("parallel::hash-multimap")
.body([] {
    auto m = parallel::HashMultiMap<HashableSerializableObj, StreamSerializable>();

    #pragma omp parallel for
    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, StreamSerializable());
    }

    MemoryOutputStreamSerializer out;
    out << m;
    out.flush();

    for (auto &x : m) {
        assert(x.k.serialized);
        assert(x.v.serialized());
    }

    parallel::HashMultiMap<HashableSerializableObj, StreamSerializable> m2;
    auto in = out.toInput();
    in >> m2;

    assert(m.size() == m2.size());

    for (auto &n : m) {
        assert(m2.contains(n.k));
        assert(m2.get(n.k).deserialized());
    }
});

// hash-set ////////////////////

#include <hash_set.h>

unit("container-serialization", "hashset<int>")
.dependsOn("hash-set")
.body([] {
    auto s = HashSet<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(i);
    }

    MemoryOutputStreamSerializer out;
    out << s;
    out.flush();

    HashSet<int> s2;
    auto in = out.toInput();
    in >> s2;

    assert(s.size() == s2.size());

    for (auto &n : s) {
        assert(s2.contains(n));
    }
});

unit("container-serialization", "hashset<hashableserializable>")
.dependsOn("hash-set")
.body([] {
    auto s = HashSet<HashableSerializableObj>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(i);
    }

    MemoryOutputStreamSerializer out;
    out << s;
    out.flush();

    for (auto &x : s) {
        assert(x.serialized);
    }

    HashSet<HashableSerializableObj> s2;
    auto in = out.toInput();
    in >> s2;

    assert(s.size() == s2.size());

    for (auto &n : s) {
        assert(s2.contains(n));
        assert(s2.get(n).deserialized);
    }
});

unit("container-serialization", "parallel::hashset<int>")
.dependsOn("parallel::hash-set")
.body([] {
    auto s = parallel::HashSet<int>();

    #pragma omp parallel for
    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(i);
    }

    MemoryOutputStreamSerializer out;
    out << s;
    out.flush();

    parallel::HashSet<int> s2;
    auto in = out.toInput();
    in >> s2;

    assert(s.size() == s2.size());

    for (auto &n : s) {
        assert(s2.contains(n));
    }
});

unit("container-serialization", "parallel::hashset<hashableserializable>")
.dependsOn("parallel::hash-set")
.body([] {
    auto s = parallel::HashSet<HashableSerializableObj>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(i);
    }

    MemoryOutputStreamSerializer out;
    out << s;
    out.flush();

    for (auto &x : s) {
        assert(x.serialized);
    }

    parallel::HashSet<HashableSerializableObj> s2;
    auto in = out.toInput();
    in >> s2;

    assert(s.size() == s2.size());

    for (auto &n : s) {
        assert(s2.contains(n));
        assert(s2.get(n).deserialized);
    }
});

unit("container-serialization", "hashmultiset<int>")
.dependsOn("hash-multiset")
.body([] {
    auto s = HashMultiSet<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(i);
    }

    MemoryOutputStreamSerializer out;
    out << s;
    out.flush();

    HashMultiSet<int> s2;
    auto in = out.toInput();
    in >> s2;

    assert(s.size() == s2.size());

    for (auto &n : s) {
        assert(s2.contains(n));
    }
});

unit("container-serialization", "hashmultiset<hashableserializable>")
.dependsOn("hash-multiset")
.body([] {
    auto s = HashMultiSet<HashableSerializableObj>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(i);
    }

    MemoryOutputStreamSerializer out;
    out << s;
    out.flush();

    for (auto &x : s) {
        assert(x.serialized);
    }

    HashMultiSet<HashableSerializableObj> s2;
    auto in = out.toInput();
    in >> s2;

    assert(s.size() == s2.size());

    for (auto &n : s) {
        assert(s2.contains(n));
        assert(s2.get(n).deserialized);
    }
});

unit("container-serialization", "parallel::hashmultiset<int>")
.dependsOn("parallel::hash-multiset")
.body([] {
    auto s = parallel::HashMultiSet<int>();

    #pragma omp parallel for
    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(i);
    }

    MemoryOutputStreamSerializer out;
    out << s;
    out.flush();

    parallel::HashMultiSet<int> s2;
    auto in = out.toInput();
    in >> s2;

    assert(s.size() == s2.size());

    for (auto &n : s) {
        assert(s2.contains(n));
    }
});

unit("container-serialization", "parallel::hashmultiset<hashableserializable>")
.dependsOn("parallel::hash-multiset")
.body([] {
    auto s = parallel::HashMultiSet<HashableSerializableObj>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(i);
    }

    MemoryOutputStreamSerializer out;
    out << s;
    out.flush();

    for (auto &x : s) {
        assert(x.serialized);
    }

    parallel::HashMultiSet<HashableSerializableObj> s2;
    auto in = out.toInput();
    in >> s2;

    assert(s.size() == s2.size());

    for (auto &n : s) {
        assert(s2.contains(n));
        assert(s2.get(n).deserialized);
    }
});

// heap ////////////////////////

#include <heap.h>

unit("container-serialization", "heap<int>")
.dependsOn("heap")
.body([] {
    auto h = Heap<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        h.push(dtest_random() * TEST_SIZE);
    }

    MemoryOutputStreamSerializer out;
    out << h;
    out.flush();

    Heap<int> h2;
    auto in = out.toInput();
    in >> h2;

    assert(h.size() == h2.size());

    for (auto it1 = h.begin(), it2 = h2.begin(); it1 != h.end(); ++it1, ++it2) {
        assert(*it1 == *it2);
    }
});

unit("container-serialization", "heap<serializable>")
.dependsOn("heap")
.body([] {
    auto h = Heap<ComparableStreamSerializable>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        h.push(ComparableStreamSerializable(i));
    }

    MemoryOutputStreamSerializer out;
    out << h;
    out.flush();

    for (auto &x : h) {
        assert(x.serialized());
    }

    Heap<ComparableStreamSerializable> h2;
    auto in = out.toInput();
    in >> h2;

    assert(h.size() == h2.size());

    for (auto &x : h2) {
        assert(x.deserialized());
    }
});
