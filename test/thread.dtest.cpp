#include <dtest.h>

#include <thread.h>
#include <list.h>

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
