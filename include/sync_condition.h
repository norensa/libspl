/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <condition_variable>
#include <mutex>
#include <exception.h>

namespace spl { 

/**
 * @brief Helper class for synchronizing concurrent actions.
 */
class SynchronizationCondition {

private:

    size_t _count;
    std::mutex _mtx;
    std::condition_variable _cv;

public:

    SynchronizationCondition()
    :   _count(0)
    { }

    SynchronizationCondition(const SynchronizationCondition &) = delete;

    SynchronizationCondition(SynchronizationCondition &&) = delete;

    ~SynchronizationCondition() = default;

    SynchronizationCondition & operator=(const SynchronizationCondition &) = delete;

    SynchronizationCondition & operator=(SynchronizationCondition &&) = delete;

    /**
     * @brief Called before an action begins.
     */
    void begin() {
        std::unique_lock lk(_mtx);
        ++_count;
    }

    /**
     * @brief Called after an action ends.
     */
    void end() {
        std::unique_lock lk(_mtx);
        if (_count == 0) throw RuntimeError("end() called without begin()");
        --_count;
        if (_count == 0) {
            lk.unlock();
            _cv.notify_all();
        }
    }

    /**
     * @brief Waits until all started actions are completed; i.e. end() is
     * called as many times as begin().
     */
    void wait() {
        std::unique_lock lk(_mtx);
        _cv.wait(lk, [this] { return _count == 0; });
    }
};

}   // namespace spl
