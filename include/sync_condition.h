/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <condition_variable>
#include <mutex>
#include <exception.h>

namespace spl { 

/**
 * @brief Helper class for synchronizing concurrent events.
 */
class SynchronizationCondition {

private:

    size_t _count;
    size_t _wakeup;
    std::mutex _mtx;
    std::condition_variable _cv;

public:

    SynchronizationCondition()
    :   _count(0),
        _wakeup(0)
    { }

    SynchronizationCondition(size_t wakeupThreshold)
    :   _count(0),
        _wakeup(wakeupThreshold)
    { }

    SynchronizationCondition(const SynchronizationCondition &) = delete;

    SynchronizationCondition(SynchronizationCondition &&) = delete;

    ~SynchronizationCondition() = default;

    SynchronizationCondition & operator=(const SynchronizationCondition &) = delete;

    SynchronizationCondition & operator=(SynchronizationCondition &&) = delete;

    /**
     * @brief Increases the internal counter by x.
     * 
     * @param x The amount to increase.
     */
    void increase(size_t x = 1) {
        std::unique_lock<std::mutex> lk(_mtx);
        _count += x;
    }

    /**
     * @brief Decreases the internal counter by x and wakes up all sleeping
     * threads in wait() if the counter is less than or equal to the wakeup
     * threshold.
     * 
     * @param x The amount to increase.
     */
    void decrease(size_t x = 1) {
        std::unique_lock<std::mutex> lk(_mtx);
        if (x > _count) throw RuntimeError("Attempt to decrease counter beyond 0");
        _count -= x;
        if (_count <= _wakeup) {
            lk.unlock();
            _cv.notify_all();
        }
    }

    /**
     * @brief Increases the internal counter by 1.
     * Note: This variant can be used to synchronize and wait for number of
     * running threads.
     */
    void begin() {
        increase(1);
    }

    /**
     * @brief Decreases the internal counter by 1.
     * Note: This variant can be used to synchronize and wait for number of
     * running threads.
     */
    void end() {
        decrease(1);
    }

    /**
     * @brief Waits until the internal counter is less than or equal to the
     * wakeup threshold.
     */
    void wait() {
        std::unique_lock<std::mutex> lk(_mtx);
        _cv.wait(lk, [this] { return _count <= _wakeup; });
    }
};

}   // namespace spl
