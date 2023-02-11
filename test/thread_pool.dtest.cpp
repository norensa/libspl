/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <thread_pool.h>
#include <chrono>

module("thread-pool")
.dependsOn({
    "exception",
    "list",
    "deque",
    "heap",
    "thread"
});

using namespace spl;

#define MANY_TASKS 20000

unit("thread-pool", "run")
.body([] {
    volatile bool ran = false;
    ThreadPool<> pool(2);
    pool.run([&ran] (ExecutionContext &) { ran = true; });
    pool.terminate();
    assert(ran);
});

perf("thread-pool", "many-tasks")
.body([] {
    ThreadPool<> pool(Thread::availableCPUs());
    for (size_t i = 0; i < MANY_TASKS; ++i) {
        pool.run([] (ExecutionContext &) { });
    }
    pool.terminate();
})
.baseline([] {
    for (size_t i = 0; i < MANY_TASKS;) {
        try {
            Thread([] { }).detach();
            ++i;
        }
        catch (...) { }
    }
});

unit("thread-pool", "ExecutionContext::wait-1")
.body([] {
    volatile int count = 0;
    ThreadPool<> pool(2);

    pool.run([&count] (ExecutionContext &ctx) {
        ctx.waitMillis(1);
        ++count;
    });
    pool.terminate();
    assert(count == 1);
});

unit("thread-pool", "ExecutionContext::wait-2")
.body([] {
    volatile int count = 0;
    ThreadPool<> pool(2);

    pool.run([&count] (ExecutionContext &ctx) {
        ctx.waitMillis(20);
        if (count == 1) ++count;
    });
    pool.run([&count] (ExecutionContext &ctx) {
        ctx.waitMillis(10);
        if (count == 0) ++count;
    });
    pool.terminate();
    assert(count == 2);
});

unit("thread-pool", "ExecutionContext::wait-3")
.body([] {
    volatile int count = 0;
    ThreadPool<> pool(1);
    pool.run([&count] (ExecutionContext &ctx) {
        ctx.waitMillis(1);
        ++count;
        ctx.waitMillis(1);
        ++count;
    });
    pool.terminate();
    assert(count == 2);
});

unit("thread-pool", "ExecutionContext::wait-4")
.body([] {
    volatile int count1 = 0, count2 = 0;
    ThreadPool<> pool(1);
    pool.run([&count1] (ExecutionContext &ctx) {
        ctx.waitMillis(1);
        ++count1;
        ctx.waitMillis(1);
        ++count1;
    });
    pool.run([&count2] (ExecutionContext &ctx) {
        ctx.waitMillis(2);
        ++count2;
    });
    pool.terminate();
    assert(count1 == 2);
    assert(count2 == 1);
});

unit("thread-pool", "ExecutionContext::suspend")
.body([] {
    volatile int count = 0;
    ThreadPool<> pool(1);

    auto f = new std::function<void(ExecutionContext &)>([&count] (ExecutionContext &ctx) {
        ++count;
        ctx.suspend();
        ++count;
    });

    pool.run(f);
    while (count == 0);

    bool resumed = false;
    do {
        try {
            pool.resume(f);
            resumed = true;
        }
        catch (...) {}
    } while (! resumed);

    while (count == 1);

    pool.terminate();

    assert(count == 2);
});

unit("thread-pool", "ExecutionContext::resched-1")
.body([] {
    volatile int count = 0;
    ThreadPool<> pool(1);
    pool.run([&count] (ExecutionContext &ctx) {
        ++count;
        if (count == 1) ctx.resched();
    });
    pool.terminate();
    assert(count == 2);
});

unit("thread-pool", "ExecutionContext::resched-2")
.body([] {
    volatile int count = 0;
    ThreadPool<> pool(1);

    auto f = new std::function<void(ExecutionContext &)>([&count] (ExecutionContext &ctx) {
        ++count;
        if (count == 1) ctx.resched(false);
    });

    pool.run(f);
    pool.run(f);
    pool.terminate();

    assert(count == 2);
});

unit("thread-pool", "randomized-single-thread")
.body([] {
    ThreadPool<> pool(1);
    for (auto i = 0; i < 1000; ++i) {
        pool.run([] (ExecutionContext &ctx) {
            auto rnd = dtest_random();
            if (rnd < 0.3) {
                auto start = std::chrono::high_resolution_clock::now();
                ctx.waitMillis(1);
                auto end = std::chrono::high_resolution_clock::now();
                assert(end - start >= std::chrono::milliseconds(1));
            }
            else if (rnd < 0.6) {
                ctx.setTimeoutMillis(1);
            }
            else if (rnd < 0.8) {
                ctx.resched();
            }
            else {
                return;
            }
        });
    }
    pool.terminate();
});

unit("thread-pool", "randomized-multi-thread")
.body([] {
    ThreadPool<> pool(4);
    for (auto i = 0; i < 1000; ++i) {
        pool.run([] (ExecutionContext &ctx) {
            auto rnd = dtest_random();
            if (rnd < 0.3) {
                auto start = std::chrono::high_resolution_clock::now();
                ctx.waitMillis(1);
                auto end = std::chrono::high_resolution_clock::now();
                assert(end - start >= std::chrono::milliseconds(1));
            }
            else if (rnd < 0.6) {
                ctx.setTimeoutMillis(1);
            }
            else if (rnd < 0.8) {
                ctx.resched();
            }
            else {
                return;
            }
        });
    }
    pool.terminate();
});
