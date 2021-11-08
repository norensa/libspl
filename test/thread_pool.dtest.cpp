#include <dtest.h>
#include <thread_pool.h>

module("thread-pool")
.dependsOn({
    "exception",
    "list",
    "deque",
    "heap",
    "thread"
});

using namespace spl;

#define MANY_TASKS 100000

unit("thread-pool", "run")
.body([] {
    volatile bool ran = false;
    ThreadPool pool(2);
    pool.run([&ran] (auto) { ran = true; });
    pool.terminate();
    assert(ran);
});

perf("thread-pool", "many-tasks")
.body([] {
    ThreadPool pool(Thread::availableCPUs() - 1);
    for (size_t i = 0; i < MANY_TASKS; ++i) {
        pool.run([] (auto) { });
    }
    pool.terminate();
})
.baseline([] {
    for (size_t i = 0; i < MANY_TASKS; ++i) {
        Thread([] { }).join();
    }
});

unit("thread-pool", "setTimeout")
.body([] {
    volatile int count = 0;
    bool defer1 = true, defer2 = true;
    ThreadPool pool(2);

    pool.run([&defer1, &defer2, &count] (ExecutionContext &ctx) {
        if (defer2) {
            defer2 = false;
            ctx.setTimeoutMillis(20);
        }
        if (count == 1) ++count;
    });
    pool.run([&defer1, &count] (ExecutionContext &ctx) {
        if (defer1) {
            defer1 = false;
            ctx.setTimeoutMillis(10);
        }
        if (count == 0) ++count;
    });
    pool.terminate();
    assert(count == 2);
});
