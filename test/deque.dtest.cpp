/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>

#include <deque.h>
#include <forward_list>
#include <thread.h>
#include <list.h>

module("deque")
.dependsOn({
    "exception"
});

module("parallel::deque")
.dependsOn({
    "deque"
});

namespace spl
{

template <typename DequeType>
struct DequeTester {
    using node_ptr = typename DequeType::node_ptr;

    static node_ptr & head(DequeType &q) {
        return q._head;
    }

    static node_ptr & tail(DequeType &q) {
        return q._tail;
    }

    static bool validTail(DequeType &q) {
        return q._tail->next == nullptr;
    }
};

namespace parallel
{
    template <typename DequeType>
    struct DequeTester {
        using node_ptr = typename DequeType::node_ptr;

        static node_ptr & head(DequeType &q) {
            return q._head;
        }

        static node_ptr & tail(DequeType &q) {
            return q._tail;
        }

        static bool validTail(DequeType &q) {
            return q._tail->next == nullptr;
        }
    };
}

} // namespace spl

using namespace spl;

#define TEST_SIZE (1024)
#define PARALLEL_TEST_SIZE (10 * 1024)
#define PERFORMANCE_TEST_SIZE (200 * 1024)
#define PERFORMANCE_MARGIN_MILLIS 10

unit("deque", "initializer-list")
.body([] {
    auto q = Deque<int>({ 1, 2, 3 });

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == 3);
    assert(q.begin() != q.end());

    auto it = q.begin();
    assert(*it++ == 1);
    assert(*it++ == 2);
    assert(*it++ == 3);
});

unit("deque", "copy")
.body([] {
    auto q = Deque<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        q.enqueue(dtest_random() * TEST_SIZE);
    }

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == TEST_SIZE);
    assert(q.begin() != q.end());

    auto l2 = q;
    for (auto it1 = q.begin(), it2 = l2.begin(); it1 != q.end(); ++it1, ++it2) {
        assert(*it1 == *it2);
    }
});

unit("deque", "move")
.body([] {
    auto q = Deque<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        q.enqueueFront(i);
    }

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == TEST_SIZE);
    assert(q.begin() != q.end());

    auto l2 = std::move(q);

    assert(q.empty());
    assert(! q.nonEmpty());
    assert(q.size() == 0);
    assert(q.begin() == q.end());
    assert(DequeTester<Deque<int>>::head(q) == nullptr);
    assert(DequeTester<Deque<int>>::tail(q) == nullptr);

    assert(! l2.empty());
    assert(l2.nonEmpty());
    assert(l2.size() == TEST_SIZE);
    assert(l2.begin() != l2.end());

    int i = TEST_SIZE - 1;
    for (auto &x : l2) {
        assert(x == i--);
    }
    assert(i == -1);
});

unit("deque", "enqueueFront")
.body([] {
    auto q = Deque<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        q.enqueueFront(i);
    }

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == TEST_SIZE);
    assert(q.begin() != q.end());
    assert(DequeTester<Deque<int>>::validTail(q));

    int i = TEST_SIZE - 1;
    for (auto &x : q) {
        assert(x == i--);
    }
    assert(i == -1);
});

unit("parallel::deque", "enqueueFront")
.body([] {
    auto q = parallel::Deque<int>();

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        q.enqueueFront(i);
    }

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == PARALLEL_TEST_SIZE);
    assert(q.begin() != q.end());
    assert(parallel::DequeTester<parallel::Deque<int>>::validTail(q));

    int sum = 0;
    for (auto &x : q) {
        sum += x;
    }
    assert(sum == PARALLEL_TEST_SIZE * (PARALLEL_TEST_SIZE - 1) / 2);
});

unit("deque", "enqueue")
.body([] {
    auto q = Deque<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        q << i;
    }

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == TEST_SIZE);
    assert(q.begin() != q.end());
    assert(DequeTester<Deque<int>>::validTail(q));

    int i = 0;
    for (auto &x : q) {
        assert(x == i++);
    }
    assert(i == TEST_SIZE);
});

unit("parallel::deque", "enqueue")
.body([] {
    auto q = parallel::Deque<int>();

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        q << i;
    }

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == PARALLEL_TEST_SIZE);
    assert(q.begin() != q.end());
    assert(parallel::DequeTester<parallel::Deque<int>>::validTail(q));

    int sum = 0;
    for (auto &x : q) {
        sum += x;
    }
    assert(sum == PARALLEL_TEST_SIZE * (PARALLEL_TEST_SIZE - 1) / 2);
});

unit("deque", "clear")
.body([] {
    auto q = Deque<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        q << i;
    }

    assert(q.size() == TEST_SIZE);

    q.clear();

    assert(q.empty());
    assert(! q.nonEmpty());
    assert(q.size() == 0);
    assert(q.begin() == q.end());
    assert(DequeTester<Deque<int>>::head(q) == nullptr);
    assert(DequeTester<Deque<int>>::tail(q) == nullptr);
});

unit("deque", "insert-before-head")
.body([] {
    auto q = Deque<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        q.insertBefore(q.begin(), i);
    }

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == TEST_SIZE);
    assert(q.begin() != q.end());
    assert(DequeTester<Deque<int>>::validTail(q));

    int i = TEST_SIZE - 1;
    for (auto &x : q) {
        assert(x == i--);
    }
    assert(i == -1);
});

unit("deque", "insert-before-tail")
.body([] {
    auto q = Deque<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        q.insertBefore(q.end(), i);
    }

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == TEST_SIZE);
    assert(q.begin() != q.end());
    assert(DequeTester<Deque<int>>::validTail(q));

    int i = 0;
    for (auto &x : q) {
        assert(x == i++);
    }
    assert(i == TEST_SIZE);
});

unit("deque", "insert-before-middle")
.body([] {
    auto q = Deque<int>();

    for (int i = 0; i < TEST_SIZE / 2; ++i) {
        q.enqueue(dtest_random() * TEST_SIZE);
    }

    auto it = q.begin();
    for (int i = 0; i < TEST_SIZE / 4; ++i) {
        ++it;
    }

    for (int i = 0; i < TEST_SIZE / 2; ++i) {
        q.insertBefore(it, dtest_random() * TEST_SIZE);
    }

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == TEST_SIZE);
    assert(q.begin() != q.end());
    assert(DequeTester<Deque<int>>::validTail(q));
});

unit("deque", "insert-after-head")
.body([] {
    auto q = Deque<int>();

    q.enqueue(TEST_SIZE - 1);

    for (int i = 0; i < TEST_SIZE - 1; ++i) {
        q.insertAfter(q.begin(), i);
    }

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == TEST_SIZE);
    assert(q.begin() != q.end());
    assert(DequeTester<Deque<int>>::validTail(q));

    int i = TEST_SIZE - 1;
    for (auto &x : q) {
        assert(x == i--);
    }
    assert(i == -1);
});

unit("deque", "insert-after-tail")
.body([] {
    auto q = Deque<int>();

    q.enqueue(0);

    try {
        q.insertAfter(q.end(), 1);
        err("Insert after a past-the-end iterator succeeded");
        assert(false);
    }
    catch (...) { }

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == 1);
    assert(q.begin() != q.end());
    assert(DequeTester<Deque<int>>::validTail(q));
    assert(DequeTester<Deque<int>>::head(q) == DequeTester<Deque<int>>::tail(q));
});

unit("deque", "insert-after-middle")
.body([] {
    auto q = Deque<int>();

    for (int i = 0; i < TEST_SIZE / 2; ++i) {
        q.enqueue(dtest_random() * TEST_SIZE);
    }

    auto it = q.begin();
    for (int i = 0; i < TEST_SIZE / 4; ++i) {
        ++it;
    }

    for (int i = 0; i < TEST_SIZE / 2; ++i) {
        q.insertAfter(it, dtest_random() * TEST_SIZE);
    }

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == TEST_SIZE);
    assert(q.begin() != q.end());
    assert(DequeTester<Deque<int>>::validTail(q));
});

unit("deque", "remove-head(new-iterator)")
.body([] {
    auto q = Deque<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        q.enqueue(dtest_random() * TEST_SIZE);
    }

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == TEST_SIZE);

    for (int i = 0; i < TEST_SIZE; ++i) {
        q.remove(q.begin());
    }

    assert(q.empty());
    assert(! q.nonEmpty());
    assert(q.size() == 0);
    assert(q.begin() == q.end());

    assert(DequeTester<Deque<int>>::head(q) == nullptr);
    assert(DequeTester<Deque<int>>::tail(q) == nullptr);
});

unit("deque", "remove-head(same-iterator)")
.body([] {
    auto q = Deque<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        q.enqueue(dtest_random() * TEST_SIZE);
    }

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == TEST_SIZE);

    auto it = q.begin();
    for (int i = 0; i < TEST_SIZE; ++i) {
        q.remove(it);
    }

    assert(it == q.end());
    assert(it == q.begin());
    assert(q.empty());
    assert(! q.nonEmpty());
    assert(q.size() == 0);

    assert(DequeTester<Deque<int>>::head(q) == nullptr);
    assert(DequeTester<Deque<int>>::tail(q) == nullptr);
});

unit("deque", "remove-middle")
.body([] {
    auto q = Deque<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        q.enqueue(dtest_random() * TEST_SIZE);
    }

    auto it = q.begin();
    for (int i = 0; i < TEST_SIZE / 2; ++i) {
        ++it;
    }

    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == TEST_SIZE);

    for (int i = 0; i < TEST_SIZE / 2; ++i) {
        q.remove(it);
    }

    assert(it == q.end());
    assert(! q.empty());
    assert(q.nonEmpty());
    assert(q.size() == TEST_SIZE / 2);
    assert(q.begin() != q.end());
    assert(DequeTester<Deque<int>>::validTail(q));
});

unit("deque", "foreach")
.body([] {
    auto q = Deque<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        q.enqueueFront(i);
    }

    int i = TEST_SIZE - 1;
    q.foreach([&i] (int x) { assert(x == i--); });
    assert(i == -1);
});

unit("deque", "map")
.body([] {
    auto q = Deque<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        q.enqueueFront(i);
    }

    auto l2 = q.map([] (int x) { return x * 2; });

    int i = TEST_SIZE - 1;
    l2.foreach([&i] (int &x) { assert(x == (i-- * 2)); });
    assert(i == -1);
});

unit("deque", "reduce")
.body([] {
    auto q = Deque<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        q.enqueueFront(i);
    }

    auto sum = q.reduce<long>([] (int x, int y) { return (long) x + y; });

    assert(typeid(sum) == typeid(long));
    assert(sum == (long)(TEST_SIZE * (TEST_SIZE - 1) / 2));

    auto sum2 = q.reduce<long>(
        [] (int x) { return (long) x; },
        [] (int x, int y) { return (long) x + y; }
    );

    assert(typeid(sum2) == typeid(long));
    assert(sum2 == sum);
});

static void producer_consumer(int numProducers, int numConsumers) {
    parallel::Deque<long> q;
    parallel::List<long> values;

    List<Thread> producers;
    for (int i = 0; i < numProducers; ++i) {
        producers.insert(Thread([&q, i, numProducers] {
            long start = PARALLEL_TEST_SIZE / numProducers * i;
            long end = (i == numProducers - 1)
                ? PARALLEL_TEST_SIZE
                : PARALLEL_TEST_SIZE / numProducers * (i + 1);

            for (long j = start; j < end; ++j) {
                q.enqueue(j);
            }
        }));
    }

    volatile bool running = true;

    List<Thread> consumers;
    for (int i = 0; i < numConsumers; ++i) {
        consumers.insert(Thread([&q, &values, &running] {
            while (running || q.nonEmpty()) {
                try {
                    values.insert(q.dequeueOrTimeout());
                }
                catch (TimeoutError &) { }
            }
        }));
    }

    producers.foreach([] (Thread &t) { t.join(); });
    running = false;
    consumers.foreach([] (Thread &t) { t.join(); });

    assert(q.size() == 0);
    assert(values.size() == PARALLEL_TEST_SIZE);

    auto sum = values.reduce<long>([] (long x, long y) { return x + y; });
    assert(sum == (PARALLEL_TEST_SIZE * (PARALLEL_TEST_SIZE - 1) / 2));
}

unit("parallel::deque", "producer-consumer(1-1)")
.dependsOn({ "parallel::list", "list", "thread" })
.body([] {
    producer_consumer(1, 1);
});

unit("parallel::deque", "producer-consumer(1-2)")
.dependsOn({ "parallel::list", "list", "thread" })
.body([] {
    producer_consumer(1, 2);
});

unit("parallel::deque", "producer-consumer(2-1)")
.dependsOn({ "parallel::list", "list", "thread" })
.body([] {
    producer_consumer(2, 1);
});

unit("parallel::deque", "producer-consumer(2-2)")
.dependsOn({ "parallel::list", "list", "thread" })
.body([] {
    producer_consumer(2, 2);
});

unit("parallel::deque", "producer-consumer(2-4)")
.dependsOn({ "parallel::list", "list", "thread" })
.body([] {
    producer_consumer(2, 4);
});

unit("parallel::deque", "producer-consumer(4-2)")
.dependsOn({ "parallel::list", "list", "thread" })
.body([] {
    producer_consumer(4, 2);
});

unit("parallel::deque", "producer-consumer(4-4)")
.dependsOn({ "parallel::list", "list", "thread" })
.body([] {
    producer_consumer(4, 4);
});
