/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>

#include <thread.h>
#include <list.h>
#include <atomic>

module("thread")
.dependsOn({
    "exception"
});

using namespace spl;

unit("thread", "create-join")
.body([] {
    bool ran = false;

    Thread([&ran] {
        ran = true;
    }).join();

    assert(ran);
});

unit("thread", "cancel")
.body([] {
    bool ran = false;

    Thread([&ran] {
        ran = true;
        while (true) {
            Thread::terminateIfCancelled();
        }
    }).cancel().join();

    assert(ran);
});

unit("thread", "detach")
.body([] {
    std::atomic<size_t> count;
    count = 1000;
    for (auto i = 0; i < 1000; ++i) {
        Thread([&count] {
            --count;
        }).detach();
    }

    while(count > 0);
    usleep(100000);     // make sure all threads have terminated
});

unit("thread", "requestTerminate")
.body([] {
    bool ran = false;

    Thread([&ran] {
        ran = true;
        while (true) {
            Thread::terminateIfRequested();
        }
    }).requestTerminate().join();

    assert(ran);
});

unit("thread", "captures")
.body([] {
    bool ran = false;
    int var = 37;

    Thread t = ([&ran, var] {
        assert(var == 37);
        ran = true;
        while (true) {
            Thread::terminateIfRequested();
        }
    });

    t.requestTerminate().join();

    assert(ran);
});

unit("thread", "list-of-threads")
.dependsOn("list")
.body([] {
    int var = 37;

    List<Thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.insert(Thread([var] {
            assert(var == 37);
            while (true) {
                Thread::terminateIfRequested();
            }
        }));
    }

    threads.foreach([] (Thread &t) { t.requestTerminate().join(); });
});
