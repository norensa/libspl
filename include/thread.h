/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <functional>
#include <pthread.h>
#include <semaphore.h>
#include <exception.h>
#include <sys/sysinfo.h>

namespace spl {

using std::move;

/**
 * @brief An error to indicate that a thread cannot be joined.
 */
struct ThreadNotJoinableError
:   Error
{
    ThreadNotJoinableError()
    :   Error("Thread is not joinable.")
    { }
};

/**
 * @brief Represents the set of CPUs a thread can run on.
 */
class ThreadAffinitySet {
    friend class Thread;

protected:
    cpu_set_t _cpus;

public:

    ThreadAffinitySet() {
        CPU_ZERO(&_cpus);
    }

    ThreadAffinitySet & add(int cpu) {
        CPU_SET(cpu, &_cpus);
        return *this;
    }

    ThreadAffinitySet & remove(int cpu) {
        CPU_CLR(cpu, &_cpus);
        return *this;
    }

    ThreadAffinitySet & clear() {
        CPU_ZERO(&_cpus);
        return *this;
    }

    bool contains(int cpu) const {
        return CPU_ISSET(cpu, &_cpus);
    }
};

/**
 * @brief Scheduling policy for threads/processes.
 */
enum class SchedulingPolicy : int {
    FIFO = SCHED_FIFO,
    ROUND_ROBIN = SCHED_RR,
    DEADLINE = SCHED_DEADLINE,
    OTHER = SCHED_OTHER,
    BATCH = SCHED_BATCH,
    IDLE = SCHED_IDLE,
};

/**
 * @brief Class for thread creation and management.
 */
class Thread {

private:

    struct Context {
        std::function<void()> func = nullptr;
        volatile bool needToTerminate = false;
        volatile bool running = false;

        Context(std::function<void()> &&func)
        :   func(move(func))
        { }
    };
    static thread_local Context *__ctx;

    Context *_ctx = nullptr;
    pthread_t _tid = 0;
    bool _joinable = false;

    static void * _start(void *ctx) {
        __ctx = (Context *) ctx;
        __ctx->running = true;
        __ctx->func();
        __ctx->running = false;
        pthread_exit(nullptr);
    }

public:

    /**
     * @brief Construct a new Thread object. This function will immediately
     * start a thread to execute the given function.
     * 
     * @param f A thread functor with return type void.
     * @param args Arguments for the thread functor.
     */
    template <typename F, typename ...Args>
    Thread(F f, Args ...args)
    :   _ctx(new Context([f, args...] { f(args...); }))
    {
        int err = pthread_create(&_tid, NULL, _start, _ctx);
        if (err != 0) {
            delete _ctx;
            throw ErrnoRuntimeError(err);
        }

        _joinable = true;
    }

    Thread(const Thread &) = delete;

    Thread(Thread &&rhs) {
        _ctx = rhs._ctx; rhs._ctx = nullptr;
        _tid = rhs._tid;
        _joinable = rhs._joinable;
    }

    ~Thread() {
        if (_ctx != nullptr) delete _ctx;
    }

    Thread & operator=(const Thread &) = delete;

    Thread & operator=(Thread &&rhs) = delete;

    /**
     * @return True if the thread can be joined, false otherwise.
     */
    bool joinable() const {
        return _joinable;
    }

    /**
     * @return True if the thread is currently running, false otherwise.
     * Note that the return value may be incorrect if the thread crashes
     * unexpectedly.
     */
    bool running() const {
        return _ctx->running;
    }

    /**
     * @brief Detaches the thread from its parent. After this call the thread
     * will no longer be joinable. The thread resources are released
     * automatically when the thread terminates.
     * 
     * @return A reference to this object for chaining.
     */
    Thread & detach() {
        if (! _joinable) throw ThreadNotJoinableError();

        int err = pthread_detach(_tid);
        if (err != 0) throw ErrnoRuntimeError(err);

        _joinable = false;

        return *this;
    }

    /**
     * @brief Attempts to join this thread or blocks until the thread can be
     * joined. Throws a ThreadNotJoinableError if the thread is not in a
     * joinable state.
     * 
     * @return A reference to this object for chaining.
     */
    Thread & join() {
        if (! _joinable) throw ThreadNotJoinableError();

        int err = pthread_join(_tid, NULL);
        if (err != 0) throw ErrnoRuntimeError(err);

        _joinable = false;
        _ctx->running = false;
        return *this;
    }

    /**
     * @brief Attempts to join this thread or timeout. Throws a
     * ThreadNotJoinableError if the thread is not in a joinable state.
     * 
     * @param timeoutNanos Timeout duration in nanoseconds.
     * @return True if the thread was actually joined, false if a timeout
     * occurred.
     */
    bool tryJoin(size_t timeoutNanos = 0) {
        if (! _joinable) throw ThreadNotJoinableError();

        int err;
        if (timeoutNanos == 0) {
            err = pthread_tryjoin_np(_tid, NULL);
        }
        else {
            struct timespec t;
            t.tv_sec = timeoutNanos / 1000000000lu;
            t.tv_nsec = timeoutNanos % 1000000000lu;
            err = pthread_timedjoin_np(_tid, NULL, &t);
        }

        if (err == 0) {
            _joinable = false;
            _ctx->running = false;
            return true;
        }
        else if (err == EBUSY || err == ETIMEDOUT) {
            return false;
        }
        else {
            throw ErrnoRuntimeError(err);
        }
    }

    /**
     * @brief Cancels this thread, terminating it at the next possible
     * cancellation point.
     * 
     * @return A reference to this object for chaining.
     */
    Thread & cancel() {
        int err = pthread_cancel(_tid);
        if (err != 0) throw ErrnoRuntimeError(err);
        return *this;
    }

    /**
     * @brief Called by a thread to check for cancellation. This creates a
     * cancellation point.
     */
    static void terminateIfCancelled() {
        pthread_testcancel();
    }

    /**
     * @brief Signals the thread that a it should terminate when possible.
     * 
     * @return A reference to this object for chaining.
     */
    Thread & requestTerminate() {
        _ctx->needToTerminate = true;
        return *this;
    }

    /**
     * @brief Called by a thread to check if a request for termination was
     * made via a call to requestTerminate() on the thread object. If such a
     * signal is found, then the thread will be terminated.
     */
    static void terminateIfRequested() {
        if (__ctx->needToTerminate) {
            __ctx->running = false;
            pthread_exit(nullptr);
        }
    }

    /**
     * @return True if a signal to request termination was made via a call to
     * requestTerminate() on the thread object, false otherwise.
     */
    static bool terminateRequested() {
        return __ctx->needToTerminate;
    }

    /**
     * @brief Sets the thread's scheduling policy and priority.
     * 
     * @param policy The desired SchedulingPolicy.
     * @param priority The desired priority value (depends on the policy).
     * @return A reference to this object for chaining.
     */
    Thread & setScheduling(SchedulingPolicy policy, int priority) {
        struct sched_param p;
        p.sched_priority = priority;

        int err = pthread_setschedparam(_tid, static_cast<int>(policy), &p);
        if (err != 0) throw ErrnoRuntimeError(err);
        return *this;
    }

    /**
     * @brief Sets the thread's scheduling policy.
     * 
     * @param policy The desired SchedulingPolicy.
     * @return A reference to this object for chaining.
     */
    Thread & setSchedulingPolicy(SchedulingPolicy policy) {
        struct sched_param p;
        int oldPolicy;

        int err = pthread_getschedparam(_tid, &oldPolicy, &p);
        if (err != 0) throw ErrnoRuntimeError(err);

        err = pthread_setschedparam(_tid, static_cast<int>(policy), &p);
        if (err != 0) throw ErrnoRuntimeError(err);

        return *this;
    }

    /**
     * @brief Sets the thread's scheduling priority.
     * 
     * @param priority The desired priority value (depends on the current
     * scheduling policy).
     * @return A reference to this object for chaining.
     */
    Thread & setSchedulingPriority(int priority) {
        struct sched_param p;
        int policy;

        int err = pthread_getschedparam(_tid, &policy, &p);
        if (err != 0) throw ErrnoRuntimeError(err);

        p.sched_priority = priority;
        err = pthread_setschedparam(_tid, policy, &p);
        if (err != 0) throw ErrnoRuntimeError(err);

        return *this;
    }

    /**
     * @brief Sets the CPU affinity of this thread.
     * 
     * @param affinitySet A reference to a ThreadAffinitySet object.
     * @return A reference to this object for chaining.
     */
    Thread & setAffinity(const ThreadAffinitySet &affinitySet) {
        int err = pthread_setaffinity_np(_tid, sizeof(cpu_set_t), &affinitySet._cpus);
        if (err != 0) throw ErrnoRuntimeError(err);
        return *this;
    }

    /**
     * @return The number of currently available CPUs in the system.
     */
    static size_t availableCPUs() {
        return get_nprocs();
    }
};

/**
 * @brief Class for counting sempahores.
 */
class Semaphore {
private:

    sem_t _sem;

public:

    /**
     * @brief Construct a new Semaphore object.
     * 
     * @param value The initial semaphore value (default = 0).
     */
    Semaphore(int32_t value = 0) {
        if (sem_init(&_sem, 0, value) != 0) {
            throw ErrnoRuntimeError();
        }
    }

    Semaphore(const Semaphore &) = delete;

    Semaphore(Semaphore &&) = delete;

    ~Semaphore() {
        sem_destroy(&_sem);
    }

    Semaphore & operator=(const Semaphore &) = delete;

    Semaphore & operator=(Semaphore &&) = delete;

    /**
     * @brief Sets the semaphore value by destroying it and reinitializing.
     * Note: this function is not thread safe.
     * 
     * @param value The desired value.
     * @return A refernce to this object for chaining.
     */
    Semaphore & operator=(int32_t value) {
        if (sem_destroy(&_sem) != 0) {
            throw ErrnoRuntimeError();
        }
        if (sem_init(&_sem, 0, value) != 0) {
            throw ErrnoRuntimeError();
        }
        return *this;
    }

    /**
     * @brief Decreases the semaphore value by 1. If the current semaphore value
     * is 0, this function will block until the semaphore value is greater than
     * 0.
     * 
     */
    void wait() {
        sem_wait(&_sem);
    }

    /**
     * @brief Decreases the semaphore value by 1. If the current semaphore value
     * is 0, this function will block until the semaphore value is greater than
     * 0, or timeout is reached.
     * 
     * @param timeoutNanos The timeout duration in nanoseconds.
     * @return True if the semaphore value was decreased, false if timeout
     * occurred.
     */
    bool wait(uint64_t timeoutNanos) {
        struct timespec t;
        t.tv_sec = timeoutNanos / 1000000000lu;
        t.tv_nsec = timeoutNanos % 1000000000lu;
        return sem_timedwait(&_sem, &t) == 0;
    }

    /**
     * @brief Decreases the semaphore value by 1. If the current semaphore value
     * is 0, this function will return immediately.
     * 
     * @return True if the semaphore value was decreased, false otherwise.
     */
    bool tryWait() {
        return sem_trywait(&_sem) == 0;
    }

    /**
     * @brief Increases the semaphore value by 1.
     */
    void notify() {
        sem_post(&_sem);
    }
};

}
