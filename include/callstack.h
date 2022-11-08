/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <string>
#include <cstring>

namespace spl {

/**
 * @brief A class representing a call stack snapshot.
*/
class CallStack {
private:
    static const int _MAX_STACK_FRAMES = 32;

    void **_stack = nullptr;
    int _len = 0;
    int _skip = 0;

    void _dispose() {
        if (_stack != nullptr) free(_stack);
    }

    void _invalidate() {
        _stack = nullptr;
        _len = 0;
        _skip = 0;
    }

    void _move(CallStack &rhs) {
        _stack = rhs._stack;
        _len = rhs._len;
        _skip = rhs._skip;
    }

    void _copy(const CallStack &rhs) {
        _len = rhs._len;
        _stack = (void **) malloc(_len * sizeof(void *));
        memcpy(_stack, rhs._stack, _len * sizeof(void *));
        _skip = rhs._skip;
    }

    /**
     * @brief Construct a new CallStack object.
     * 
     * @param len Number of callstack framesin the buffer.
     * @param stack The callsatack buffer.
     * @param skip The number of frames to ignore.
     */
    CallStack(int len, void **stack, int skip)
    :   _stack(stack),
        _len(len),
        _skip(skip)
    { }

public:

    CallStack(const CallStack &rhs) {
        _copy(rhs);
    }

    CallStack(CallStack &&rhs) {
        _move(rhs);
        rhs._invalidate();
    }

    /**
     * @brief Produces a call stack snapshot instance.
     * 
     * @param[in] skip the number of stack frames to not include. By default the
     * trace() function will not include itself. In addition a few more stack
     * frames may be omitted, if needed.
     * @return A Callstack instance.
     */
    static CallStack trace(int skip = 0);

    ~CallStack() {
        _dispose();
        _invalidate();
    }

    CallStack & operator=(const CallStack &rhs) {
        if (this != &rhs) {
            _dispose();
            _copy(rhs);
        }
        return *this;
    }

    CallStack & operator=(CallStack &&rhs) {
        if (this != &rhs) {
            _dispose();
            _move(rhs);
            rhs._invalidate();
        }
        return *this;
    }

    /**
     * @return A string representation of this callstack instance.
     */
    std::string toString() const noexcept;

    /**
     * @return A pointer to the internal callstack addresses.
     */
    void * const * stack() const {
        return _stack + _skip;
    }

    /**
     * @return The number of callstack frames.
     */
    size_t size() const {
        return _len - _skip;
    }
};

}
