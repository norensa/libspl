/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <thread.h>
#include <deque.h>
#include <list.h>
#include <exception.h>
#include <chrono>
#include <functional>
#include <setjmp.h>
#include <unistd.h>
#include <heap.h>

namespace spl {

/**
 * @brief Generic thread context class. This is used as the context object for
 * tasks running under a ThreadPool.
 */
class ExecutionContext {

    template <typename Task, typename Context>
    friend class ThreadPool;

private:

    enum class Status : int {
        COMPLETED,
        DEFERRED,
        RESCHED,
        WILL_RESCHED,
    };

    Status _status;
    jmp_buf _ret;
    std::chrono::high_resolution_clock::time_point _deferTime;
    void *_task;

    ExecutionContext() = default;

    template <typename Task>
    void _run(Task *t) {
        _task = t;
        _status = Status::COMPLETED;
        if (setjmp(_ret) == 0) {
            (*t)(*this);
        }
    }

protected:

    template <typename Task>
    Task * currentTask() const {
        return static_cast<Task *>(_task);
    }

public:

    /**
     * @brief Reschedules this thread after a timeout duration has passed.
     * 
     * @param duration The desired timeout duration.
     */
    void setTimeout(const std::chrono::duration<uint64_t, std::nano> &duration) {
        _deferTime = std::chrono::high_resolution_clock::now() + duration;
        _status = Status::DEFERRED;
        longjmp(_ret, 1);
    }

    /**
     * @brief Reschedules this thread after a timeout duration has passed.
     * 
     * @param nanos The desired timeout duration in nanoseconds.
     */
    void setTimeoutNanos(uint64_t nanos) {
        setTimeout(std::chrono::nanoseconds(nanos));
    }

    /**
     * @brief Reschedules this thread after a timeout duration has passed.
     * 
     * @param micros The desired timeout duration in microseconds.
     */
    void setTimeoutMicros(uint64_t micros) {
        setTimeout(std::chrono::microseconds(micros));
    }

    /**
     * @brief Reschedules this thread after a timeout duration has passed.
     * 
     * @param millis The desired timeout duration in milliseconds.
     */
    void setTimeoutMillis(uint64_t millis) {
        setTimeout(std::chrono::milliseconds(millis));
    }

    /**
     * @brief Reschedules this thread after a timeout duration has passed.
     * 
     * @param seconds The desired timeout duration in seconds.
     */
    void setTimeout(uint64_t seconds) {
        setTimeout(std::chrono::seconds());
    }

    /**
     * @brief Reschedules this thread by inserting to the back of the ready
     * queue.
     */
    void resched() {
        _status = Status::RESCHED;
        longjmp(_ret, 1);
    }

    void markNotDone() {
        _status = Status::WILL_RESCHED;
    }
};

/**
 * @brief An error to indicate that scheduling a task was rejected.
 */
class TaskRejectedError
:   public Error
{

public:

    TaskRejectedError()
    :   Error("Task scheduling rejected")
    { }
};

/**
 * @brief Class for thread pooling and task scheduling.
 * 
 * @tparam context_type The thread context type. This type have ExecutionContext
 * as a base class. Default = ExecutionContext.
 * @tparam task_type The task type implementing the member function
 * `void operator()(context_type &)`.
 * Default = std::function<void(ExecutionContext &)>.
 */
template <
    typename context_type = ExecutionContext,
    typename task_type = std::function<void(ExecutionContext &)>
>
class ThreadPool {

public:

    using Task = task_type;
    using Context = context_type;

private:

    List<Thread> _threads;
    parallel::Deque<Task *> _tasks;
    volatile bool _running = false;
    volatile bool _stopping = false;
    uint64_t _dequeueTimeout = 10000UL;

    template <typename Task>
    struct DeferredTask {
        Task *task;
        std::chrono::high_resolution_clock::time_point time;

        bool operator<(const DeferredTask &rhs) const {
            return time > rhs.time;
        }
    };

    void _worker()  {
        Context ctx;
        Heap<DeferredTask<Task>> deferredTasks;

        while (_running || deferredTasks.nonEmpty()) {
            Task *task = nullptr;

            if (deferredTasks.nonEmpty()) {
                auto now = std::chrono::high_resolution_clock::now();
                auto next = deferredTasks.top().time;
                if (now >= next) {
                    task = deferredTasks.pop().task;
                }
                else {
                    try {
                        task = _tasks.dequeueOrTimeout((next - now).count());
                    } catch(DequeueTimedout &) { }
                }
            }
            else {
                task = _tasks.dequeue();
            }

            if (task != nullptr) {
                ctx._run(task);

                switch (ctx._status) {
                case Context::Status::COMPLETED:
                    delete task;
                    break;

                case Context::Status::DEFERRED:
                    deferredTasks.push({ task, ctx._deferTime });
                    break;

                case Context::Status::RESCHED:
                    _tasks.enqueue(task);
                    break;

                case Context::Status::WILL_RESCHED:
                    break;

                default:
                    delete task;
                }
            }
            else {
                sched_yield();
            }
        }
    }

public:

    /**
     * @brief Construct a new ThreadPool object.
     * 
     * @param size The number of threads in the thread pool.
     */
    ThreadPool(size_t size) {
        _running = true;
        for (size_t i = 0; i < size; ++i) {
            _threads.insert(
                Thread([this] () { _worker(); })
            );
        }
    }

    ThreadPool(const ThreadPool &) = delete;

    ThreadPool(ThreadPool &&) = delete;

    ~ThreadPool() = default;

    ThreadPool & operator=(const ThreadPool &) = delete;

    ThreadPool & operator=(ThreadPool &&) = delete;

    /**
     * @brief Enqueues a task to the back of the ready queue. Throws a
     * TaskRejectedError if the thread pool is being terminated.
     * 
     * @param t The task to enqueue.
     * @throws TaskRejectedError if the thread pool is being terminated.
     */
    void run(Task *t) {
        if (_stopping) throw TaskRejectedError();
        _tasks.enqueue(t);
    }

    /**
     * @brief Enqueues a task to the back of the ready queue. Throws a
     * TaskRejectedError if the thread pool is being terminated.
     * 
     * @param t The task to enqueue.
     * @throws TaskRejectedError if the thread pool is being terminated.
     */
    void run(const Task &t) {
        run(new Task(t));
    }

    /**
     * @brief Enqueues a task to the back of the ready queue. Throws a
     * TaskRejectedError if the thread pool is being terminated.
     * 
     * @param t The task to enqueue.
     * @throws TaskRejectedError if the thread pool is being terminated.
     */
    void run(Task &&t) {
        run(new Task(std::move(t)));
    }

    /**
     * @brief Terminates the thread pool. This function sisables enqueueing new
     * tasks and waits for all tasks to finish. Throws a TimeoutError if the
     * thread pool cannot terminate before the indicated timeout duration.
     * 
     * @param timeoutMillis The timeout duration in milliseconds. Default =
     * 1000ms.
     * @throws TimeoutError if the thread pool cannot terminate before the
     * indicated timeout duration.
     */
    void terminate(uint64_t timeoutMillis = 1000)  {
        auto now = std::chrono::high_resolution_clock::now();
        auto timeout = now + std::chrono::milliseconds(timeoutMillis);

        _stopping = true;

        while (now <= timeout && _tasks.nonEmpty()) {
            usleep(50);
            now = std::chrono::high_resolution_clock::now();
        }

        if (now > timeout) {
            _stopping = false;
            throw TimeoutError();
        }

        timeout = now + std::chrono::milliseconds(timeoutMillis);

        _running = false;

        for (size_t i = 0; i < _threads.size(); ++i) {
            _tasks.enqueue(nullptr);
        }

        _threads.foreach([this, &timeout] (Thread &t) {
            if (t.joinable()) {
                do {
                    if (std::chrono::high_resolution_clock::now() > timeout) {
                        _stopping = false;
                        throw TimeoutError();
                    }

                    if (_tasks.empty()) _tasks.enqueue(nullptr);
                } while (! t.tryJoin(_dequeueTimeout));
            }
            else if (t.running()) {
                _stopping = false;
                throw RuntimeError("Failed to join one or more threads");
            }
        });

        _tasks.foreach([] (Task *t) { 
            if (t != nullptr) delete t;
        });
        _tasks.clear();
    }
};

}   // namespace spl
