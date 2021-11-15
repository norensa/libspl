#include <dtest.h>
#include <hash_set.h>
#include <unordered_set>
#include <list.h>

module("hash-set")
.dependsOn({
    "exception"
});

module("parallel::hash-set")
.dependsOn({
    "hash-set"
});

module("hash-multiset")
.dependsOn({
    "hash-set" 
});

module("parallel::hash-multiset")
.dependsOn({ 
    "hash-multiset",
    "parallel::hash-set"
});

using namespace spl;

#define TEST_SIZE (1024)
#define PARALLEL_TEST_SIZE (10 * 1024)
#define PERFORMANCE_TEST_SIZE (200 * 1024)
#define PERFORMANCE_MARGIN_MILLIS 10

namespace spl
{

template <typename HashSetType>
struct HashSetTester {
    static size_t tableSize(HashSetType &m) {
        return m._controller.tableSize;
    }

    static size_t bucketSize(HashSetType &m) {
        return m._controller.bucketSize;
    }

    static size_t numBuckets(HashSetType &m) {
        return m._controller.nBuckets;
    }
};

namespace parallel
{
    template <typename HashSetType>
    struct HashSetTester {
        static size_t tableSize(HashSetType &m) {
            return m._controller.tableSize;
        }

        static size_t bucketSize(HashSetType &m) {
            return m._controller.bucketSize;
        }

        static size_t numBuckets(HashSetType &m) {
            return m._controller.nBuckets;
        }
    };
}

}

struct X : Hashable {
    long v;
    void *buf;

    X()
    :   v(-1),
        buf(nullptr)
    { }

    X(long v)
    :   v(v),
        buf(malloc(1))
    { }

    X(const X &r)
    :   v(r.v),
        buf(malloc(1))
    { }

    X(X &&r)
    :   v(r.v),
        buf(r.buf)
    {
        r.buf = nullptr;
    }

    ~X() {
        if (buf != nullptr) {
            free(buf);
            buf = nullptr;
        }
    }

    X & operator=(const X &r) {
        if (this != &r) {
            if (buf != nullptr) free(buf);
            v = r.v;
            buf = malloc(1);
        }
        return *this;
    }

    X & operator=(X &&r) {
        if (this != &r) {
            if (buf != nullptr) free(buf);
            v = r.v;
            buf = r.buf;
            r.buf = nullptr;
        }
        return *this;
    }

    size_t hash() const {
        return (size_t) v;
    }

    bool operator==(const X &rhs) const {
        return v == rhs.v;
    }
};

unit("hash-set", "initializer-list")
.body([] {

    HashSet<X> s({ 0, 1, 2 });

    assert(s.size() == 3);
    assert((HashSetTester<HashSet<X>>::tableSize(s)) <= 2 * 3);
    assert((HashSetTester<HashSet<X>>::bucketSize(s)) <= 2);

    for (int i = 0; i < 3; ++i) {
        assert(s.contains(i));
    }

    size_t count = 0;
    for (auto &x : s) {
        assert(x.v < 3);
        ++count;
    }
    assert(count == 3);
});

unit("hash-set", "unique-dense-keys")
.body([] {

    HashSet<X> s;

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(i);
    }

    assert(s.size() == TEST_SIZE);
    assert((HashSetTester<HashSet<X>>::tableSize(s)) <= 2 * TEST_SIZE);
    assert((HashSetTester<HashSet<X>>::bucketSize(s)) <= 2);

    for (int i = 0; i < TEST_SIZE; ++i) {
        assert(s.contains(i));
    }

    size_t count = 0;
    for (auto &x : s) {
        assert(x.v < TEST_SIZE);
        ++count;
    }
    assert(count == TEST_SIZE);
});

unit("parallel::hash-set", "unique-dense-keys")
.body([] {

    parallel::HashSet<X> s;

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        s.put(i);
    }

    assert(s.size() == PARALLEL_TEST_SIZE);
    assert((parallel::HashSetTester<parallel::HashSet<X>>::tableSize(s)) <= 2 * PARALLEL_TEST_SIZE);

    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        assert(s.contains(i));
    }

    size_t count = 0;
    for (auto &x : s) {
        assert(x.v < PARALLEL_TEST_SIZE);
        ++count;
    }
    assert(count == PARALLEL_TEST_SIZE);
});

unit("hash-set", "dense-keys")
.body([] {

    HashSet<X> s;

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(dtest_random() * TEST_SIZE);
    }

    size_t count = 0;
    for (auto &x : s) {
        assert(x.v <= TEST_SIZE);
        ++count;
    }
    assert(count <= TEST_SIZE);
});

unit("hash-set", "sparse-keys")
.body([] {

    HashSet<X> s;

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(dtest_random() * (TEST_SIZE * 1000));
    }

    size_t count = 0;
    for (auto &x : s) {
        assert(x.v <= TEST_SIZE * 1000);
        ++count;
    }
    assert(count <= TEST_SIZE);
});

unit("hash-set", "colliding-keys")
.body([] {

    HashSet<X> s;

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(dtest_random() * (TEST_SIZE / 10));
    }

    size_t count = 0;
    for (auto &x : s) {
        assert(x.v <= TEST_SIZE / 10);
        ++count;
    }
    assert(count <= TEST_SIZE);
});

unit("hash-set", "foreach")
.body([] {

    HashSet<X> s;

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(i);
    }

    assert(s.size() == TEST_SIZE);

    size_t count = 0;
    s.foreach([&count] (X &n) {
        assert(n.v < TEST_SIZE);
        ++count;
    });
    assert(count == TEST_SIZE);
});

unit("hash-set", "map")
.body([] {

    HashSet<X> s;

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(i);
    }

    assert(s.size() == TEST_SIZE);

    auto s2 = s.map([] (const X &n) -> X {
        return { n.v + 1 };
    });

    assert(s2.size() == TEST_SIZE);

    assert((HashSetTester<HashSet<X>>::tableSize(s2)) <= (HashSetTester<HashSet<X>>::tableSize(s)));
    assert((HashSetTester<HashSet<X>>::bucketSize(s2)) <= (HashSetTester<HashSet<X>>::bucketSize(s)));

    for (int i = 1; i <= TEST_SIZE; ++i) {
        assert(s2.contains(i));
    }

    size_t count = 0;
    for (auto &x : s2) {
        assert(x.v <= TEST_SIZE);
        ++count;
    }
    assert(count == TEST_SIZE);
});

unit("hash-set", "map-to-list")
.dependsOn("list")
.body([] {

    HashSet<X> s;

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(i);
    }

    assert(s.size() == TEST_SIZE);

    auto l = s.map<List<X>>([] (const X &n) -> X {
        return n;
    });

    assert(l.size() == TEST_SIZE);

    size_t count = 0;
    for (auto &x : l) {
        assert(s.contains(x));
        ++count;
    }
    assert(count == TEST_SIZE);
});

unit("hash-set", "to-list")
.dependsOn("list")
.body([] {

    HashSet<X> s;

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(i);
    }

    assert(s.size() == TEST_SIZE);

    auto l = s.to<List<X>>();

    assert(l.size() == TEST_SIZE);

    size_t count = 0;
    for (auto &x : l) {
        assert(s.contains(x));
        ++count;
    }
    assert(count == TEST_SIZE);
});

perf("hash-set", "put(p)")
.performanceMarginMillis(PERFORMANCE_MARGIN_MILLIS)
.body([] {

    HashSet<int> s;

    for (int i = 0; i < PERFORMANCE_TEST_SIZE; ++i) {
        s.put(i);
    }
})
.baseline([] {

    std::unordered_set<int> m;

    for (int i = 0; i < PERFORMANCE_TEST_SIZE; ++i) {
        m.insert(i);
    }
});

unit("hash-multiset", "unique-dense-keys")
.body([] {

    HashMultiSet<X> s;

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(i);
    }

    assert(s.size() == TEST_SIZE);
    assert((HashSetTester<HashMultiSet<X>>::tableSize(s)) <= 2 * TEST_SIZE);
    assert((HashSetTester<HashMultiSet<X>>::bucketSize(s)) <= 2);

    for (int i = 0; i < TEST_SIZE; ++i) {
        assert(s.contains(i));
    }

    size_t count = 0;
    for (auto &x : s) {
        assert(x.v < TEST_SIZE);
        ++count;
    }
    assert(count == TEST_SIZE);
});

unit("parallel::hash-multiset", "unique-dense-keys")
.body([] {

    parallel::HashMultiSet<X> s;

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        s.put(i);
    }

    assert(s.size() == PARALLEL_TEST_SIZE);
    assert((parallel::HashSetTester<parallel::HashMultiSet<X>>::tableSize(s)) <= 2 * PARALLEL_TEST_SIZE);

    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        assert(s.contains(i));
    }

    size_t count = 0;
    for (auto &x : s) {
        assert(x.v < PARALLEL_TEST_SIZE);
        ++count;
    }
    assert(count == PARALLEL_TEST_SIZE);
});

unit("hash-multiset", "dense-keys")
.body([] {

    HashMultiSet<X> s;

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(dtest_random() * TEST_SIZE);
    }

    assert(s.size() == TEST_SIZE);

    size_t count = 0;
    for (auto &x : s) {
        assert(x.v <= TEST_SIZE);
        ++count;
    }
    assert(count == TEST_SIZE);
});

unit("parallel::hash-multiset", "dense-keys")
.body([] {

    parallel::HashMultiSet<X> s;

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        s.put(dtest_random() * PARALLEL_TEST_SIZE);
    }

    assert(s.size() == PARALLEL_TEST_SIZE);

    size_t count = 0;
    for (auto &x : s) {
        assert(x.v <= PARALLEL_TEST_SIZE);
        ++count;
    }
    assert(count == PARALLEL_TEST_SIZE);
});

unit("hash-multiset", "sparse-keys")
.body([] {

    HashMultiSet<X> s;

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(dtest_random() * (TEST_SIZE * 1000));
    }

    assert(s.size() == TEST_SIZE);

    size_t count = 0;
    for (auto &x : s) {
        assert(x.v <= TEST_SIZE * 1000);
        ++count;
    }
    assert(count == TEST_SIZE);
});

unit("parallel::hash-multiset", "sparse-keys")
.body([] {

    parallel::HashMultiSet<X> s;

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        s.put(dtest_random() * (PARALLEL_TEST_SIZE * 1000));
    }

    assert(s.size() == PARALLEL_TEST_SIZE);

    size_t count = 0;
    for (auto &x : s) {
        assert(x.v <= PARALLEL_TEST_SIZE * 1000);
        ++count;
    }
    assert(count == PARALLEL_TEST_SIZE);
});

unit("hash-multiset", "colliding-keys")
.body([] {

    HashMultiSet<X> s;

    for (int i = 0; i < TEST_SIZE; ++i) {
        s.put(dtest_random() * (TEST_SIZE / 10));
    }

    assert(s.size() == TEST_SIZE);

    size_t count = 0;
    for (auto &x : s) {
        assert(x.v <= TEST_SIZE / 10);
        ++count;
    }
    assert(count == TEST_SIZE);
});

unit("parallel::hash-multiset", "colliding-keys")
.body([] {

    parallel::HashMultiSet<X> s;

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        s.put(dtest_random() * (PARALLEL_TEST_SIZE / 10));
    }

    assert(s.size() == PARALLEL_TEST_SIZE);

    size_t count = 0;
    for (auto &x : s) {
        assert(x.v <= PARALLEL_TEST_SIZE / 10);
        ++count;
    }
    assert(count == PARALLEL_TEST_SIZE);
});
