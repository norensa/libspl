/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <hash_map.h>
#include <unordered_map>
#include <list.h>
#include "test_hashable.cpp"

module("hash-map")
.dependsOn({
    "exception"
});

module("parallel::hash-map")
.dependsOn({
    "hash-map"
});

module("hash-multimap")
.dependsOn({
    "hash-map" 
});

module("parallel::hash-multimap")
.dependsOn({ 
    "hash-multimap",
    "parallel::hash-map"
});

using namespace spl;

#define TEST_SIZE (1024)
#define PARALLEL_TEST_SIZE (10 * 1024)
#define PERFORMANCE_TEST_SIZE (200 * 1024)
#define PERFORMANCE_MARGIN_MILLIS 10

namespace spl
{

template <typename HashMapType>
struct HashMapTester {
    static size_t tableSize(HashMapType &m) {
        return m._controller.tableSize;
    }

    static size_t bucketSize(HashMapType &m) {
        return m._controller.bucketSize;
    }

    static size_t numBuckets(HashMapType &m) {
        return m._controller.nBuckets;
    }
};

namespace parallel
{
    template <typename HashMapType>
    struct HashMapTester {
        static size_t tableSize(HashMapType &m) {
            return m._controller.tableSize;
        }

        static size_t bucketSize(HashMapType &m) {
            return m._controller.bucketSize;
        }

        static size_t numBuckets(HashMapType &m) {
            return m._controller.nBuckets;
        }
    };
}

}

unit("hash-map", "initializer-list")
.body([] {

    HashMap<HashableObj, HashableObj> m({
        { 0, 0 },
        { 1, 2 },
        { 2, 4 }
    });

    assert(m.size() == 3);
    assert((HashMapTester<HashMap<HashableObj, HashableObj>>::tableSize(m)) <= 2 * 3);
    assert((HashMapTester<HashMap<HashableObj, HashableObj>>::bucketSize(m)) <= 2);

    for (int i = 0; i < 3; ++i) {
        assert(m.get(i).v == i * 2);
        assert(m[i].v == m.get(i).v)
    }

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k.v < 3);
        assert(x.k.v * 2 == x.v.v);
        ++count;
    }
    assert(count == 3);
});

unit("hash-map", "unique-dense-keys")
.body([] {

    HashMap<HashableObj, HashableObj> m;

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, i * 2);
    }

    assert(m.size() == TEST_SIZE);
    assert((HashMapTester<HashMap<HashableObj, HashableObj>>::tableSize(m)) <= 2 * TEST_SIZE);
    assert((HashMapTester<HashMap<HashableObj, HashableObj>>::bucketSize(m)) <= 2);

    for (int i = 0; i < TEST_SIZE; ++i) {
        assert(m.get(i).v == i * 2);
        assert(m[i].v == m.get(i).v)
    }

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k.v < TEST_SIZE);
        assert(x.k.v * 2 == x.v.v);
        ++count;
    }
    assert(count == TEST_SIZE);
});

unit("parallel::hash-map", "unique-dense-keys")
.body([] {
    parallel::HashMap<HashableObj, HashableObj> m;

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        m.put(i, i * 2);
    }

    assert(m.size() == PARALLEL_TEST_SIZE);
    assert((parallel::HashMapTester<parallel::HashMap<HashableObj, HashableObj>>::tableSize(m)) <= 2 * PARALLEL_TEST_SIZE);

    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        assert(m.get(i).v == i * 2);
        assert(m[i].v == m.get(i).v)
    }

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k.v < PARALLEL_TEST_SIZE);
        assert(x.k.v * 2 == x.v.v);
        ++count;
    }
    assert(count == PARALLEL_TEST_SIZE);
});

unit("parallel::hash-map", "unique-dense-keys-2")
.body([] {
    parallel::HashMap<int, int> m;

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        m.put(i, i * 2);
    }

    assert(m.size() == PARALLEL_TEST_SIZE);

    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        assert(m.get(i) == i * 2);
        assert(m[i] == m.get(i));
    }

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k < PARALLEL_TEST_SIZE);
        assert(x.k * 2 == x.v);
        ++count;
    }
    assert(count == PARALLEL_TEST_SIZE);
});

unit("hash-map", "dense-keys")
.body([] {

    HashMap<HashableObj, HashableObj> m;

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(dtest_random() * TEST_SIZE, i * 2);
    }

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k.v <= TEST_SIZE);
        ++count;
    }
    assert(count <= TEST_SIZE);
});

unit("hash-map", "sparse-keys")
.body([] {

    HashMap<HashableObj, HashableObj> m;

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(dtest_random() * (TEST_SIZE * 1000), i * 2);
    }

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k.v <= TEST_SIZE * 1000);
        ++count;
    }
    assert(count <= TEST_SIZE);
});

unit("hash-map", "colliding-keys")
.body([] {

    HashMap<HashableObj, HashableObj> m;

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(dtest_random() * (TEST_SIZE / 10), i * 2);
    }

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k.v <= TEST_SIZE / 10);
        ++count;
    }
    assert(count <= TEST_SIZE);
});


unit("hash-map", "foreach")
.body([] {

    HashMap<HashableObj, HashableObj> m;

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, i * 2);
    }

    assert(m.size() == TEST_SIZE);

    size_t count = 0;
    m.foreach([&count] (MapNode<HashableObj, HashableObj> &n) {
        assert(n.k.v < TEST_SIZE);
        assert(n.k.v * 2 == n.v.v);
        ++count;
    });
    assert(count == TEST_SIZE);
});

unit("hash-map", "map")
.body([] {

    HashMap<HashableObj, HashableObj> m;

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, i * 2);
    }

    assert(m.size() == TEST_SIZE);

    auto m2 = m.map([] (const MapNode<HashableObj, HashableObj> &n) -> MapNode<HashableObj, HashableObj> {
        return { n.k.v + 1, n.v.v * 2 };
    });

    assert(m2.size() == TEST_SIZE);

    assert((HashMapTester<HashMap<HashableObj, HashableObj>>::tableSize(m2)) <= (HashMapTester<HashMap<HashableObj, HashableObj>>::tableSize(m)));
    assert((HashMapTester<HashMap<HashableObj, HashableObj>>::bucketSize(m2)) <= (HashMapTester<HashMap<HashableObj, HashableObj>>::bucketSize(m)));

    for (int i = 0; i < TEST_SIZE; ++i) {
        assert(m2.get(i + 1).v == i * 4);
    }

    size_t count = 0;
    for (auto &x : m2) {
        assert(x.k.v <= TEST_SIZE);
        assert((x.k.v - 1) * 4 == x.v.v);
        ++count;
    }
    assert(count == TEST_SIZE);
});

unit("hash-map", "map-to-list")
.dependsOn("list")
.body([] {

    HashMap<HashableObj, HashableObj> m;

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, i * 2);
    }

    assert(m.size() == TEST_SIZE);

    auto l = m.map<List<HashableObj>>([] (const MapNode<HashableObj, HashableObj> &n) -> HashableObj {
        return n.k;
    });

    assert(l.size() == TEST_SIZE);

    size_t count = 0;
    for (auto &x : l) {
        assert(m.contains(x));
        ++count;
    }
    assert(count == TEST_SIZE);
});

unit("hash-map", "to-list")
.dependsOn("list")
.body([] {

    HashMap<HashableObj, HashableObj> m;

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, i * 2);
    }

    assert(m.size() == TEST_SIZE);

    auto l = m.to<List<MapNode<HashableObj, HashableObj>>>();

    assert(l.size() == TEST_SIZE);

    size_t count = 0;
    for (auto &x : l) {
        assert(m.contains(x.k));
        ++count;
    }
    assert(count == TEST_SIZE);
});

perf("hash-map", "put(p)")
.performanceMarginMillis(PERFORMANCE_MARGIN_MILLIS)
.body([] {

    HashMap<int, int> m;

    for (int i = 0; i < PERFORMANCE_TEST_SIZE; ++i) {
        m.put(i, i * 2);
    }
})
.baseline([] {

    std::unordered_map<int, int> m;

    for (int i = 0; i < PERFORMANCE_TEST_SIZE; ++i) {
        m.insert({i, i * 2});
    }
});

unit("hash-multimap", "unique-dense-keys")
.body([] {

    HashMultiMap<HashableObj, HashableObj> m;

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(i, i * 2);
    }

    assert(m.size() == TEST_SIZE);
    assert((HashMapTester<HashMultiMap<HashableObj, HashableObj>>::tableSize(m)) <= 2 * TEST_SIZE);
    assert((HashMapTester<HashMultiMap<HashableObj, HashableObj>>::bucketSize(m)) <= 2);

    for (int i = 0; i < TEST_SIZE; ++i) {
        assert(m.get(i).v == i * 2);
    }

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k.v < TEST_SIZE);
        assert(x.k.v * 2 == x.v.v);
        ++count;
    }
    assert(count == TEST_SIZE);
});

unit("parallel::hash-multimap", "unique-dense-keys")
.body([] {

    parallel::HashMultiMap<HashableObj, HashableObj> m;

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        m.put(i, i * 2);
    }

    assert(m.size() == PARALLEL_TEST_SIZE);
    assert((parallel::HashMapTester<parallel::HashMultiMap<HashableObj, HashableObj>>::tableSize(m)) <= 2 * PARALLEL_TEST_SIZE);

    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        assert(m.get(i).v == i * 2);
    }

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k.v < PARALLEL_TEST_SIZE);
        assert(x.k.v * 2 == x.v.v);
        ++count;
    }
    assert(count == PARALLEL_TEST_SIZE);
});

unit("parallel::hash-multimap", "unique-dense-keys-2")
.body([] {
    parallel::HashMultiMap<int, int> m;

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        m.put(i, i * 2);
    }

    assert(m.size() == PARALLEL_TEST_SIZE);

    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        assert(m.get(i) == i * 2);
    }

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k < PARALLEL_TEST_SIZE);
        assert(x.k * 2 == x.v);
        ++count;
    }
    assert(count == PARALLEL_TEST_SIZE);
});

unit("hash-multimap", "dense-keys")
.body([] {

    HashMultiMap<HashableObj, HashableObj> m;

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(dtest_random() * TEST_SIZE, i * 2);
    }

    assert(m.size() == TEST_SIZE);

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k.v <= TEST_SIZE);
        ++count;
    }
    assert(count == TEST_SIZE);
});

unit("parallel::hash-multimap", "dense-keys")
.body([] {

    parallel::HashMultiMap<HashableObj, HashableObj> m;

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        m.put(dtest_random() * PARALLEL_TEST_SIZE, i * 2);
    }

    assert(m.size() == PARALLEL_TEST_SIZE);

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k.v <= PARALLEL_TEST_SIZE);
        ++count;
    }
    assert(count == PARALLEL_TEST_SIZE);
});

unit("hash-multimap", "sparse-keys")
.body([] {

    HashMultiMap<HashableObj, HashableObj> m;

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(dtest_random() * (TEST_SIZE * 1000), i * 2);
    }

    assert(m.size() == TEST_SIZE);

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k.v <= TEST_SIZE * 1000);
        ++count;
    }
    assert(count == TEST_SIZE);
});

unit("parallel::hash-multimap", "sparse-keys")
.body([] {

    parallel::HashMultiMap<HashableObj, HashableObj> m;

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        m.put(dtest_random() * (PARALLEL_TEST_SIZE * 1000), i * 2);
    }

    assert(m.size() == PARALLEL_TEST_SIZE);

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k.v <= PARALLEL_TEST_SIZE * 1000);
        ++count;
    }
    assert(count == PARALLEL_TEST_SIZE);
});

unit("hash-multimap", "colliding-keys")
.body([] {

    HashMultiMap<HashableObj, HashableObj> m;

    for (int i = 0; i < TEST_SIZE; ++i) {
        m.put(dtest_random() * (TEST_SIZE / 10), i * 2);
    }

    assert(m.size() == TEST_SIZE);

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k.v <= TEST_SIZE / 10);
        ++count;
    }
    assert(count == TEST_SIZE);
});

unit("parallel::hash-multimap", "colliding-keys")
.body([] {

    parallel::HashMultiMap<HashableObj, HashableObj> m;

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        m.put(dtest_random() * (PARALLEL_TEST_SIZE / 10), i * 2);
    }

    assert(m.size() == PARALLEL_TEST_SIZE);

    size_t count = 0;
    for (auto &x : m) {
        assert(x.k.v <= PARALLEL_TEST_SIZE / 10);
        ++count;
    }
    assert(count == PARALLEL_TEST_SIZE);
});
