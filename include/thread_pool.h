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
#include <hash_set.h>
#include <ucontext.h>

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
        COMPLETE,
        RESCHED,
        RESCHED_LATER,
        DEFERRED,
        DEFERRED_SAVED,
        SUSPENDED,
    };

    // per-context object
    size_t _stackSize;
    ucontext_t _ctx;

    // per-task
    ucontext_t *_uctx;
    volatile Status _status;
    std::chrono::high_resolution_clock::time_point _deferTime;

    // per-thread
    static thread_local void *_task;
    static thread_local ExecutionContext *_this;

    template <typename Task>
    static void _ret() {
        (*static_cast<Task *>(_task))(*_this);
        _this->_status = Status::COMPLETE;
        // return to parent context
    }

    template <typename Task>
    void _run(Task *t, ucontext_t *uctx, void *stack = nullptr) {
        // set per-task and per-thread members
        _uctx = uctx;
        _task = t;
        _this = this;

        // initialize context stack, if a stack is given
        if (stack != nullptr) {
            // current context
            getcontext(_uctx);

            // new stack
            _uctx->uc_stack.ss_sp = stack;
            _uctx->uc_stack.ss_size = _stackSize;
            _uctx->uc_link = &_ctx;

            // jump into _ret on first swap
            makecontext(_uctx, _ret<Task>, 0);
        }

        // swap into task
        swapcontext(&_ctx, _uctx);
    }

protected:

    ExecutionContext() = default;

    template <typename Task>
    Task * currentTask() const {
        return static_cast<Task *>(_task);
    }

public:

    /**
     * @brief Reschedules this task. This restarts the execution of the task from
     * the begining.
     * 
     * @param insert Indicates whether to re-insert the task to the back of the
     * ready queue. If the task is not inserted, then it must be re-run using
     * ThreadPool:run(Task *). (default = true)
     */
    void resched(bool insert = true) {
        _status = insert ? Status::RESCHED : Status::RESCHED_LATER;
        // jump into _run, never to return
        setcontext(&_ctx);
    }

    /**
     * @brief Pauses this task until a timeout duration has passed. The actual
     * wait duration is arbitrarily long but guaranteed to be at least the
     * specified amount. This wil restart the task from the beginning.
     * 
     * @param duration The desired wait duration.
     */
    void setTimeout(const std::chrono::duration<uint64_t, std::nano> &duration) {
        _deferTime = std::chrono::high_resolution_clock::now() + duration;
        _status = Status::DEFERRED;
        // jump into _run, never to return
        setcontext(&_ctx);
    }

    /**
     * @brief Pauses this task until a timeout duration has passed. The actual
     * wait duration is arbitrarily long but guaranteed to be at least the
     * specified amount. This wil restart the task from the beginning.
     * 
     * @param nanos The desired wait duration in nanoseconds.
     */
    void setTimeoutNanos(uint64_t nanos) {
        wait(std::chrono::nanoseconds(nanos));
    }

    /**
     * @brief Pauses this task until a timeout duration has passed. The actual
     * wait duration is arbitrarily long but guaranteed to be at least the
     * specified amount. This wil restart the task from the beginning.
     * 
     * @param micros The desired wait duration in microseconds.
     */
    void setTimeoutMicros(uint64_t micros) {
        wait(std::chrono::microseconds(micros));
    }

    /**
     * @brief Pauses this task until a timeout duration has passed. The actual
     * wait duration is arbitrarily long but guaranteed to be at least the
     * specified amount. This wil restart the task from the beginning.
     * 
     * @param millis The desired wait duration in milliseconds.
     */
    void setTimeoutMillis(uint64_t millis) {
        wait(std::chrono::milliseconds(millis));
    }

    /**
     * @brief Pauses this task until a timeout duration has passed. The actual
     * wait duration is arbitrarily long but guaranteed to be at least the
     * specified amount. This wil restart the task from the beginning.
     * 
     * @param seconds The desired wait duration in seconds.
     */
    void setTimeout(uint64_t seconds) {
        setTimeout(std::chrono::seconds(seconds));
    }

    /**
     * @brief Pauses this task until a timeout duration has passed. The actual
     * wait duration is arbitrarily long but guaranteed to be at least the
     * specified amount. Unlike setTimeout, this resumes execution of the task.
     * 
     * @param duration The desired wait duration.
     */
    void wait(const std::chrono::duration<uint64_t, std::nano> &duration) {
        _deferTime = std::chrono::high_resolution_clock::now() + duration;
        _status = Status::DEFERRED_SAVED;
        // swap into _run
        swapcontext(_uctx, &_ctx);
        // wait is over, go back to task
    }

    /**
     * @brief Pauses this task until a timeout duration has passed. The actual
     * wait duration is arbitrarily long but guaranteed to be at least the
     * specified amount. Unlike setTimeout, this resumes execution of the task.
     * 
     * @param nanos The desired wait duration in nanoseconds.
     */
    void waitNanos(uint64_t nanos) {
        wait(std::chrono::nanoseconds(nanos));
    }

    /**
     * @brief Pauses this task until a timeout duration has passed. The actual
     * wait duration is arbitrarily long but guaranteed to be at least the
     * specified amount. Unlike setTimeout, this resumes execution of the task.
     * 
     * @param micros The desired wait duration in microseconds.
     */
    void waitMicros(uint64_t micros) {
        wait(std::chrono::microseconds(micros));
    }

    /**
     * @brief Pauses this task until a timeout duration has passed. The actual
     * wait duration is arbitrarily long but guaranteed to be at least the
     * specified amount. Unlike setTimeout, this resumes execution of the task.
     * 
     * @param millis The desired wait duration in milliseconds.
     */
    void waitMillis(uint64_t millis) {
        wait(std::chrono::milliseconds(millis));
    }

    /**
     * @brief Pauses this task until a timeout duration has passed. The actual
     * wait duration is arbitrarily long but guaranteed to be at least the
     * specified amount. Unlike setTimeout, this resumes execution of the task.
     * 
     * @param seconds The desired wait duration in seconds.
     */
    void wait(uint64_t seconds) {
        wait(std::chrono::seconds(seconds));
    }

    /**
     * @brief Suspends this task indefinitely until execution is resumed using
     * ThreadPool::resume(Task *).
     */
    void suspend() {
        _status = Status::SUSPENDED;
        // swap into _run
        swapcontext(_uctx, &_ctx);
        // suspension is over, go back to task
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

    static constexpr size_t DEFAULT_STACK_SIZE = 16 * 1024;

private:

    struct QueuedTask {
        Task *task = nullptr;
        ucontext_t *uctx = nullptr;
        void *stack = nullptr;

        QueuedTask() = default;

        QueuedTask(Task *task)
        :   task(task),
            uctx(nullptr),
            stack(nullptr)
        { }

        QueuedTask(Task *task, ucontext_t *uctx, void *stack)
        :   task(task),
            uctx(uctx),
            stack(stack)
        { }

        bool saveContext(ucontext_t *u, void *s) {
            if (uctx == nullptr) {
                uctx = u;
                stack = s;
                return true;
            }
            return false;
        }

        bool hasContext() {
            return uctx != nullptr;
        }

        void freeContext() {
            if (stack != nullptr) free(stack);
            if (uctx != nullptr) delete uctx;
        }

        void freeAll() {
            delete task;
            freeContext();
        }
    };

    struct QueuedTaskHash {
        size_t operator()(const QueuedTask &queuedTask) const {
            return (size_t) queuedTask.task;
        }

        size_t operator()(const Task *task) const {
            return (size_t) task;
        }
    };

    struct QueuedTaskEqual {
        bool operator()(const QueuedTask &lhs, const QueuedTask &rhs) const {
            return lhs.task == rhs.task;
        }

        bool operator()(const QueuedTask &lhs, const Task *rhs) const {
            return lhs.task == rhs;
        }

        bool operator()(const Task *lhs, const QueuedTask &rhs) const {
            return lhs == rhs.task;
        }
    };

    List<Thread> _threads;
    size_t _stackSize;
    parallel::Deque<QueuedTask> _tasks;
    parallel::HashMultiSet<QueuedTask, QueuedTaskHash, QueuedTaskEqual> _suspendedTasks;
    volatile bool _running = false;
    volatile bool _stopping = false;
    uint64_t _dequeueTimeout = 10000UL;

    template <typename Task>
    struct DeferredTask : QueuedTask {
        std::chrono::high_resolution_clock::time_point time;

        DeferredTask() = default;

        DeferredTask(QueuedTask t, std::chrono::high_resolution_clock::time_point time)
        :   QueuedTask(t),
            time(time)
        { }

        bool operator<(const DeferredTask &rhs) const {
            return time > rhs.time;
        }
    };

    void _worker() {
        Heap<DeferredTask<Task>> deferredTasks;

        Context ctx;
        ctx._stackSize = _stackSize;

        ucontext_t *uctx = new ucontext_t();
        void *stack = malloc(_stackSize);

        QueuedTask qt;

        while (_running || deferredTasks.nonEmpty()) {
            // check deferred tasks first
            if (deferredTasks.nonEmpty()) {
                auto now = std::chrono::high_resolution_clock::now();

                // time of the next task
                auto next = deferredTasks.top().time;

                // if the time of the next task is up
                if (now >= next) {
                    // then run the next defferred task
                    qt = deferredTasks.pop();
                }
                else {
                    // otherwise wait on the ready queue
                    // If something shows up on the ready queue between now and
                    // the time of the next task, it gets priority
                    try {
                        qt = _tasks.dequeueOrTimeout((next - now).count());
                    } catch(TimeoutError &) {
                        // set null to indicate "no task"
                        qt.task = nullptr;
                    }
                }
            }
            else {
                // no deferred tasks, get something from the ready queue or wait
                qt = _tasks.dequeue();
            }

            // if we got a task
            if (qt.task != nullptr) {

                if (qt.hasContext()) {
                    // use the task's saved context and stack
                    ctx._run(qt.task, qt.uctx);
                }
                else {
                    // use the current context and stack
                    ctx._run(qt.task, uctx, stack);
                }

                switch (ctx._status) {
                case Context::Status::COMPLETE:
                    // release all task resources
                    qt.freeAll();
                break;

                case Context::Status::RESCHED:
                    // no longer needed, execution will start from the begining
                    qt.freeContext();
                    // re-enqueue to the back of the ready queue
                    _tasks.enqueue(qt.task);
                break;

                case Context::Status::RESCHED_LATER:
                    // no longer needed, execution will start from the begining
                    qt.freeContext();
                break;

                case Context::Status::DEFERRED:
                    deferredTasks.push({ qt, ctx._deferTime });
                break;

                case Context::Status::DEFERRED_SAVED:
                    // if the task didn't already have a saved context,
                    // then steal current context & stack and keep them for the task
                    if (qt.saveContext(uctx, stack)) {
                        // allocate new context & stack for the next task
                        uctx = new ucontext_t();
                        stack = malloc(_stackSize);
                    }
                    deferredTasks.push({ qt, ctx._deferTime });
                break;

                case Context::Status::SUSPENDED:
                    // if the task didn't already have a saved context,
                    // then steal current context & stack and keep them for the task
                    if (qt.saveContext(uctx, stack)) {
                        // allocate new context & stack for the next task
                        uctx = new ucontext_t();
                        stack = malloc(_stackSize);
                    }
                    _suspendedTasks.put(qt);
                break;
                }
            }
        }

        delete uctx;
        free(stack);
    }

public:

    /**
     * @brief Construct a new ThreadPool object.
     * 
     * @param size The number of threads in the thread pool.
     * @param stackSize The size of the stack used for tasks. (default = 16 KiB)
     */
    ThreadPool(size_t size, size_t stackSize = DEFAULT_STACK_SIZE)
    :   _stackSize(stackSize)
    {
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
     * @brief Resumes a suspended task. The task should be previously submitted
     * using run(Task *) as this pointer is used to lookup the suspended task.
     * 
     * @param t The task to resume.
     */
    void resume(Task *t) {
        _tasks.enqueue(_suspendedTasks.remove(t));
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

        _tasks.foreach([] (QueuedTask &t) { 
            if (t.task != nullptr) t.freeAll();
        });
        _tasks.clear();
    }
};

}   // namespace spl
