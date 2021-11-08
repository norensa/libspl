#include <dtest.h>
#include <heap.h>

using namespace spl;

#define TEST_SIZE (1024)

unit("heap", "initializer-list")
.body([] {
    auto h = Heap<int>({ 1, 2, 3 });

    assert(! h.empty());
    assert(h.nonEmpty());
    assert(h.size() == 3);
    assert(h.begin() != h.end());
});

unit("heap", "push-pop")
.body([] {
    auto h = Heap<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        h.push(i);
    }

    assert(! h.empty());
    assert(h.nonEmpty());
    assert(h.size() == TEST_SIZE);
    assert(h.begin() != h.end());

    int last = TEST_SIZE;
    for (size_t i = 0; i < TEST_SIZE; ++i) {
        auto x = h.pop();
        assert(x == last - 1);
        last = x;
    }

    assert(h.empty());
    assert(! h.nonEmpty());
    assert(h.size() == 0);
    assert(h.begin() == h.end());
});

unit("heap", "top")
.body([] {
    auto h = Heap<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        h.push(i);
    }

    assert(! h.empty());
    assert(h.nonEmpty());
    assert(h.size() == TEST_SIZE);
    assert(h.begin() != h.end());

    int last = TEST_SIZE;
    for (size_t i = 0; i < TEST_SIZE; ++i) {
        auto x = h.top();
        assert(x == last - 1);
        last = x;
        h.pop();
    }

    assert(h.empty());
    assert(! h.nonEmpty());
    assert(h.size() == 0);
    assert(h.begin() == h.end());
});

unit("heap", "copy")
.body([] {
    auto h = Heap<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        h.push(i);
    }

    assert(! h.empty());
    assert(h.nonEmpty());
    assert(h.size() == TEST_SIZE);
    assert(h.begin() != h.end());

    auto h2 = h;
    for (auto it1 = h.begin(), it2 = h2.begin(); it1 != h.end(); ++it1, ++it2) {
        assert(*it1 == *it2);
    }

    int last = TEST_SIZE;
    for (size_t i = 0; i < TEST_SIZE; ++i) {
        auto x = h.pop();
        assert(x == last - 1);
        last = x;
    }

    assert(h.empty());
    assert(! h.nonEmpty());
    assert(h.size() == 0);
    assert(h.begin() == h.end());
});

unit("heap", "move")
.body([] {
    auto h = Heap<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        h.push(i);
    }

    assert(! h.empty());
    assert(h.nonEmpty());
    assert(h.size() == TEST_SIZE);
    assert(h.begin() != h.end());

    auto h2 = move(h);

    assert(h.empty());
    assert(! h.nonEmpty());
    assert(h.size() == 0);
    assert(h.begin() == h.end());

    assert(! h2.empty());
    assert(h2.nonEmpty());
    assert(h2.size() == TEST_SIZE);
    assert(h2.begin() != h2.end());

    int last = TEST_SIZE;
    for (size_t i = 0; i < TEST_SIZE; ++i) {
        auto x = h2.pop();
        assert(x == last - 1);
        last = x;
    }

    assert(h.empty());
    assert(! h.nonEmpty());
    assert(h.size() == 0);
    assert(h.begin() == h.end());
});
