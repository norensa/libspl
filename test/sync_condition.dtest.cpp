/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <sync_condition.h>
#include <thread.h>
#include <atomic>

module("sync-condition")
.dependsOn({
    "exception"
});

using namespace spl;

unit("sync-condition", "wait")
.dependsOn("thread")
.body([] {
    SynchronizationCondition cond;

    for (auto i = 0; i < 1000; ++i) cond.begin();

    std::atomic<size_t> count;
    count = 1000;
    for (auto i = 0; i < 1000; ++i) {
        Thread([&count, &cond] {
            --count;
            cond.end();
        }).detach();
    }

    cond.wait();
    assert(count == 0);
    usleep(100000);     // make sure all threads have terminated
});
