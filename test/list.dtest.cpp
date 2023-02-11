/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>

#include <list.h>
#include <forward_list>
#include <list>

module("list")
.dependsOn({
    "exception"
});

module("parallel::list")
.dependsOn({
    "list"
});

namespace spl
{

template <typename ListType>
struct ListTester {
    using node_ptr = typename ListType::node_ptr;

    static node_ptr & head(ListType &l) {
        return l._head;
    }

    static node_ptr & tail(ListType &l) {
        return l._tail;
    }

    static bool validTail(ListType &l) {
        return l._tail->next == nullptr;
    }
};

namespace parallel
{
    template <typename ListType>
    struct ListTester {
        using node_ptr = typename ListType::node_ptr;

        static node_ptr & head(ListType &l) {
            return l._head;
        }

        static node_ptr & tail(ListType &l) {
            return l._tail;
        }

        static bool validTail(ListType &l) {
            return l._tail.load()->next == nullptr;
        }
    };
}

} // namespace spl

using namespace spl;

#define TEST_SIZE (1024)
#define PARALLEL_TEST_SIZE (10 * 1024)
#define PERFORMANCE_TEST_SIZE (400 * 1024)
#define PERFORMANCE_MARGIN_MILLIS 10

unit("list", "initializer-list")
.body([] {
    auto l = List<int>({ 1, 2, 3 });

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == 3);
    assert(l.begin() != l.end());

    auto it = l.begin();
    assert(*it++ == 1);
    assert(*it++ == 2);
    assert(*it++ == 3);
});

unit("list", "copy")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.insert(dtest_random() * TEST_SIZE);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == TEST_SIZE);
    assert(l.begin() != l.end());

    auto l2 = l;
    for (auto it1 = l.begin(), it2 = l2.begin(); it1 != l.end(); ++it1, ++it2) {
        assert(*it1 == *it2);
    }
});

unit("list", "move")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        i >> l;
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == TEST_SIZE);
    assert(l.begin() != l.end());

    auto l2 = std::move(l);

    assert(l.empty());
    assert(! l.nonEmpty());
    assert(l.size() == 0);
    assert(l.begin() == l.end());
    assert(ListTester<List<int>>::head(l) == nullptr);
    assert(ListTester<List<int>>::tail(l) == nullptr);

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

unit("list", "prepend")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        i >> l;
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == TEST_SIZE);
    assert(l.begin() != l.end());
    assert(ListTester<List<int>>::validTail(l));

    int i = TEST_SIZE - 1;
    for (auto &x : l) {
        assert(x == i--);
    }
    assert(i == -1);
});

unit("parallel::list", "prepend")
.body([] {
    auto l = parallel::List<int>();

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        i >> l;
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == PARALLEL_TEST_SIZE);
    assert(l.begin() != l.end());
    assert(parallel::ListTester<parallel::List<int>>::validTail(l));

    int sum = 0;
    for (auto &x : l) {
        sum += x;
    }
    assert(sum == PARALLEL_TEST_SIZE * (PARALLEL_TEST_SIZE - 1) / 2);
});

perf("list", "prepend(p)")
.performanceMarginMillis(PERFORMANCE_MARGIN_MILLIS)
.body([] {
    auto l = List<int>();
    for (int i = 0; i < PERFORMANCE_TEST_SIZE; ++i) {
        i >> l;
    }
})
.baseline([] {
    auto l = std::forward_list<int>();
    for (int i = 0; i < PERFORMANCE_TEST_SIZE; ++i) {
        l.push_front(i);
    }
});

unit("list", "append")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l << i;
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == TEST_SIZE);
    assert(l.begin() != l.end());
    assert(ListTester<List<int>>::validTail(l));

    int i = 0;
    for (auto &x : l) {
        assert(x == i++);
    }
    assert(i == TEST_SIZE);
});

unit("parallel::list", "append")
.body([] {
    auto l = parallel::List<int>();

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        l << i;
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == PARALLEL_TEST_SIZE);
    assert(l.begin() != l.end());
    assert(parallel::ListTester<parallel::List<int>>::validTail(l));

    int sum = 0;
    for (auto &x : l) {
        sum += x;
    }
    assert(sum == PARALLEL_TEST_SIZE * (PARALLEL_TEST_SIZE - 1) / 2);
});

perf("list", "append(p)")
.performanceMarginMillis(PERFORMANCE_MARGIN_MILLIS)
.body([] {
    auto l = List<int>();
    for (int i = 0; i < PERFORMANCE_TEST_SIZE; ++i) {
        l << i;
    }
})
.baseline([] {
    auto l = std::list<int>();
    for (int i = 0; i < PERFORMANCE_TEST_SIZE; ++i) {
        l.push_back(i);
    }
});

unit("list", "clear")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l << i;
    }

    assert(l.size() == TEST_SIZE);

    l.clear();

    assert(l.empty());
    assert(! l.nonEmpty());
    assert(l.size() == 0);
    assert(l.begin() == l.end());
    assert(ListTester<List<int>>::head(l) == nullptr);
    assert(ListTester<List<int>>::tail(l) == nullptr);
});

unit("list", "insert")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.insert(dtest_random() * TEST_SIZE);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == TEST_SIZE);
    assert(l.begin() != l.end());
    assert(ListTester<List<int>>::validTail(l));
});

unit("parallel::list", "insert")
.body([] {
    auto l = parallel::List<int>();

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        l.insert(i);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == PARALLEL_TEST_SIZE);
    assert(l.begin() != l.end());
    assert(parallel::ListTester<parallel::List<int>>::validTail(l));

    int sum = 0;
    for (auto &x : l) {
        sum += x;
    }
    assert(sum == PARALLEL_TEST_SIZE * (PARALLEL_TEST_SIZE - 1) / 2);
});

unit("list", "insert-before-head")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.insertBefore(l.begin(), i);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == TEST_SIZE);
    assert(l.begin() != l.end());
    assert(ListTester<List<int>>::validTail(l));

    int i = TEST_SIZE - 1;
    for (auto &x : l) {
        assert(x == i--);
    }
    assert(i == -1);
});

unit("parallel::list", "insert-before-head")
.body([] {
    auto l = parallel::List<int>();

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        l.insertBefore(l.begin(), i);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == PARALLEL_TEST_SIZE);
    assert(l.begin() != l.end());
    assert(parallel::ListTester<parallel::List<int>>::validTail(l));

    int sum = 0;
    for (auto &x : l) {
        sum += x;
    }
    assert(sum == PARALLEL_TEST_SIZE * (PARALLEL_TEST_SIZE - 1) / 2);
});

unit("list", "insert-before-tail")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.insertBefore(l.end(), i);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == TEST_SIZE);
    assert(l.begin() != l.end());
    assert(ListTester<List<int>>::validTail(l));

    int i = 0;
    for (auto &x : l) {
        assert(x == i++);
    }
    assert(i == TEST_SIZE);
});

unit("parallel::list", "insert-before-tail")
.body([] {
    auto l = parallel::List<int>();

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        l.insertBefore(l.end(), i);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == PARALLEL_TEST_SIZE);
    assert(l.begin() != l.end());
    assert(parallel::ListTester<parallel::List<int>>::validTail(l));

    int sum = 0;
    for (auto &x : l) {
        sum += x;
    }
    assert(sum == PARALLEL_TEST_SIZE * (PARALLEL_TEST_SIZE - 1) / 2);
});

unit("list", "insert-before-middle")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE / 2; ++i) {
        l.insert(dtest_random() * TEST_SIZE);
    }

    auto it = l.begin();
    for (int i = 0; i < TEST_SIZE / 4; ++i) {
        ++it;
    }

    for (int i = 0; i < TEST_SIZE / 2; ++i) {
        l.insertBefore(it, dtest_random() * TEST_SIZE);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == TEST_SIZE);
    assert(l.begin() != l.end());
    assert(ListTester<List<int>>::validTail(l));
});

unit("parallel::list", "insert-before-middle")
.body([] {
    auto l = parallel::List<int>();

    for (int i = 0; i < PARALLEL_TEST_SIZE / 2; ++i) {
        l.insert(i);
    }

    auto it = l.begin();
    for (int i = 0; i < PARALLEL_TEST_SIZE / 4; ++i) {
        ++it;
    }

    #pragma omp parallel for
    for (int i = PARALLEL_TEST_SIZE / 2; i < PARALLEL_TEST_SIZE; ++i) {
        l.insertBefore(it, i);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == PARALLEL_TEST_SIZE);
    assert(l.begin() != l.end());
    assert(parallel::ListTester<parallel::List<int>>::validTail(l));

    int sum = 0;
    for (auto &x : l) {
        sum += x;
    }
    assert(sum == PARALLEL_TEST_SIZE * (PARALLEL_TEST_SIZE - 1) / 2);
});

unit("list", "insert-after-head")
.body([] {
    auto l = List<int>();

    l.insert(TEST_SIZE - 1);

    for (int i = 0; i < TEST_SIZE - 1; ++i) {
        l.insertAfter(l.begin(), i);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == TEST_SIZE);
    assert(l.begin() != l.end());
    assert(ListTester<List<int>>::validTail(l));

    int i = TEST_SIZE - 1;
    for (auto &x : l) {
        assert(x == i--);
    }
    assert(i == -1);
});

unit("parallel::list", "insert-after-head")
.body([] {
    auto l = parallel::List<int>();

    l.insert(0);

    #pragma omp parallel for
    for (int i = 1; i < PARALLEL_TEST_SIZE; ++i) {
        l.insertAfter(l.begin(), i);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == PARALLEL_TEST_SIZE);
    assert(l.begin() != l.end());
    assert(parallel::ListTester<parallel::List<int>>::validTail(l));

    int sum = 0;
    for (auto &x : l) {
        sum += x;
    }
    assert(sum == PARALLEL_TEST_SIZE * (PARALLEL_TEST_SIZE - 1) / 2);
});

unit("list", "insert-after-tail")
.body([] {
    auto l = List<int>();

    l.insert(0);

    try {
        l.insertAfter(l.end(), 1);
        err("Insert after a past-the-end iterator succeeded");
        assert(false);
    }
    catch (...) { }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == 1);
    assert(l.begin() != l.end());
    assert(ListTester<List<int>>::validTail(l));
    assert(ListTester<List<int>>::head(l) == ListTester<List<int>>::tail(l));
});

unit("list", "insert-after-middle")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE / 2; ++i) {
        l.insert(dtest_random() * TEST_SIZE);
    }

    auto it = l.begin();
    for (int i = 0; i < TEST_SIZE / 4; ++i) {
        ++it;
    }

    for (int i = 0; i < TEST_SIZE / 2; ++i) {
        l.insertAfter(it, dtest_random() * TEST_SIZE);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == TEST_SIZE);
    assert(l.begin() != l.end());
    assert(ListTester<List<int>>::validTail(l));
});

unit("parallel::list", "insert-before-middle")
.body([] {
    auto l = parallel::List<int>();

    for (int i = 0; i < PARALLEL_TEST_SIZE / 2; ++i) {
        l.insert(i);
    }

    auto it = l.begin();
    for (int i = 0; i < PARALLEL_TEST_SIZE / 4; ++i) {
        ++it;
    }

    #pragma omp parallel for
    for (int i = PARALLEL_TEST_SIZE / 2; i < PARALLEL_TEST_SIZE; ++i) {
        l.insertAfter(it, i);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == PARALLEL_TEST_SIZE);
    assert(l.begin() != l.end());
    assert(parallel::ListTester<parallel::List<int>>::validTail(l));

    int sum = 0;
    for (auto &x : l) {
        sum += x;
    }
    assert(sum == PARALLEL_TEST_SIZE * (PARALLEL_TEST_SIZE - 1) / 2);
});

unit("list", "remove-head(new-iterator)")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.insert(dtest_random() * TEST_SIZE);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == TEST_SIZE);

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.remove(l.begin());
    }

    assert(l.empty());
    assert(! l.nonEmpty());
    assert(l.size() == 0);
    assert(l.begin() == l.end());

    assert(ListTester<List<int>>::head(l) == nullptr);
    assert(ListTester<List<int>>::tail(l) == nullptr);
});

unit("parallel::list", "remove-head(new-iterator)")
.body([] {
    auto l = parallel::List<int>();

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        l.insert(dtest_random() * PARALLEL_TEST_SIZE);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == PARALLEL_TEST_SIZE);

    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        l.remove(l.begin());
    }

    assert(l.empty());
    assert(! l.nonEmpty());
    assert(l.size() == 0);
    assert(l.begin() == l.end());

    assert(parallel::ListTester<parallel::List<int>>::head(l) == nullptr);
    assert(parallel::ListTester<parallel::List<int>>::tail(l) == nullptr);
});

unit("list", "remove-head(same-iterator)")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.insert(dtest_random() * TEST_SIZE);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == TEST_SIZE);

    auto it = l.begin();
    for (int i = 0; i < TEST_SIZE; ++i) {
        l.remove(it);
    }

    assert(it == l.end());
    assert(it == l.begin());
    assert(l.empty());
    assert(! l.nonEmpty());
    assert(l.size() == 0);

    assert(ListTester<List<int>>::head(l) == nullptr);
    assert(ListTester<List<int>>::tail(l) == nullptr);
});

unit("parallel::list", "remove-head(same-iterator)")
.body([] {
    auto l = parallel::List<int>();

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        l.insert(dtest_random() * PARALLEL_TEST_SIZE);
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == PARALLEL_TEST_SIZE);

    auto it = l.begin();
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        l.remove(it);
    }

    assert(it == l.end());
    assert(it == l.begin());
    assert(l.empty());
    assert(! l.nonEmpty());
    assert(l.size() == 0);

    assert(parallel::ListTester<parallel::List<int>>::head(l) == nullptr);
    assert(parallel::ListTester<parallel::List<int>>::tail(l) == nullptr);
});

unit("list", "remove-middle")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        l.insert(dtest_random() * TEST_SIZE);
    }

    auto it = l.begin();
    for (int i = 0; i < TEST_SIZE / 2; ++i) {
        ++it;
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == TEST_SIZE);

    for (int i = 0; i < TEST_SIZE / 2; ++i) {
        l.remove(it);
    }

    assert(it == l.end());
    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == TEST_SIZE / 2);
    assert(l.begin() != l.end());
    assert(ListTester<List<int>>::validTail(l));
});

unit("parallel::list", "remove-middle")
.body([] {
    auto l = parallel::List<int>();

    #pragma omp parallel for
    for (int i = 0; i < PARALLEL_TEST_SIZE; ++i) {
        l.insert(dtest_random() * TEST_SIZE);
    }

    auto it = l.begin();
    for (int i = 0; i < PARALLEL_TEST_SIZE / 2; ++i) {
        ++it;
    }

    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == PARALLEL_TEST_SIZE);

    for (int i = 0; i < PARALLEL_TEST_SIZE / 2; ++i) {
        l.remove(it);
    }

    assert(it == l.end());
    assert(! l.empty());
    assert(l.nonEmpty());
    assert(l.size() == PARALLEL_TEST_SIZE / 2);
    assert(l.begin() != l.end());
    assert(parallel::ListTester<parallel::List<int>>::validTail(l));
});

unit("list", "foreach")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        i >> l;
    }

    int i = TEST_SIZE - 1;
    l.foreach([&i] (int x) { assert(x == i--); });
    assert(i == -1);
});

unit("list", "map")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        i >> l;
    }

    auto l2 = l.map([] (int x) { return x * 2; });

    int i = TEST_SIZE - 1;
    l2.foreach([&i] (int &x) { assert(x == (i-- * 2)); });
    assert(i == -1);
});

unit("list", "reduce")
.body([] {
    auto l = List<int>();

    for (int i = 0; i < TEST_SIZE; ++i) {
        i >> l;
    }

    auto sum = l.reduce<long>([] (int x, int y) { return (long) x + y; });

    assert(typeid(sum) == typeid(long));
    assert(sum == (long)(TEST_SIZE * (TEST_SIZE - 1) / 2));

    auto sum2 = l.reduce<long>(
        [] (int x) { return (long) x; },
        [] (int x, int y) { return (long) x + y; }
    );

    assert(typeid(sum2) == typeid(long));
    assert(sum2 == sum);
});
