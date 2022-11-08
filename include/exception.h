/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <callstack.h>
#include <string>
#include <cstring>
#include <iostream>
#include <util.h>

namespace spl {

namespace core {

inline const char *__strerror(int err = errno) {
    return strerror(err);
}

}

/**
 * @brief Generic error class supporting compile-time error messages.
*/
class Error
:   public std::exception
{
protected:

    const char *_msg;

public:

    /**
     * @brief Construct a new Error object.
     * 
     * @param msg The error message. Note that this must be a const char * that
     * is valid throughout the entire lifetime of the Error object.
     */
    Error(const char *msg)
    :   _msg(msg)
    { }

    /**
     * @return The error message as a C-style string.
     */
    const char * what() const noexcept override { 
        return _msg;
    }

    /**
     * @brief Prints the error message to cerr.
     */
    void print() const noexcept {
        std::cerr << what();
    }
};

/**
 * @brief Generic error class supporting run-time error messages.
*/
class DynamicMessageError
:   public Error {
private:

    std::string _msgStr;

public:

    /**
     * @brief Construct a new DynamicMessageError object.
     * 
     * @param msg One or more messages to be concatenated.
     */
    template <typename ...Msg>
    DynamicMessageError(const Msg &...msg)
    :   Error(nullptr)
    {
        _msgStr = make_str(msg...);
        _msg = _msgStr.c_str();
    }
};

/**
 * @brief Base class of all callstack-traceable errors.
*/
class TraceableError
:   public Error
{
protected:

    CallStack _callstack;
    const char *_type;
    const char * _function;
    const char * _file;
    int _line;

    bool _msgPrepared = false;

public:

    /**
     * @brief Construct a new TraceableError object.
     * 
     * @param type The type of the error.
     * @param msg The error message.
     * @param function The name of the function at which the error occurred.
     * @param file The name of the source file at which the error occurred.
     * @param line The line number at which the error occurred.
     */
    TraceableError(
        const char *type,
        const char *msg,
        const char *function,
        const char *file,
        int line
    ):  Error(msg),
        _callstack(CallStack::trace(1)),
        _type(type),
        _function(function),
        _file(file),
        _line(line)
    { }

    TraceableError(const TraceableError &rhs)
    :   Error(rhs._msg),
        _callstack(rhs._callstack),
        _type(rhs._type),
        _function(rhs._function),
        _file(rhs._file),
        _line(rhs._line),
        _msgPrepared(rhs._msgPrepared)
    {
        if (rhs._msgPrepared) _msg = strdup(rhs._msg);
    }

    TraceableError(TraceableError &&rhs)
    :   Error(rhs._msg),
        _callstack(std::move(rhs._callstack)),
        _type(rhs._type),
        _function(rhs._function),
        _file(rhs._file),
        _line(rhs._line),
        _msgPrepared(rhs._msgPrepared)
    {
        if (rhs._msgPrepared) rhs._msgPrepared = false;
    }

    ~TraceableError() {
        if (_msgPrepared) free((void *) _msg);
    }

    TraceableError & operator=(const TraceableError &rhs) {
        if (this != &rhs) {
            if (_msgPrepared) free((void *) _msg);

            _msg = rhs._msg;
            _callstack = rhs._callstack;
            _type = rhs._type;
            _function = rhs._function;
            _file = rhs._file;
            _line = rhs._line;
            _msgPrepared = rhs._msgPrepared;

            if (_msgPrepared) _msg = strdup(_msg);
        }
        return *this;
    }

    TraceableError & operator=(TraceableError &&rhs) {
        if (this != &rhs) {
            if (_msgPrepared) free((void *) _msg);

            _msg = rhs._msg;
            _callstack = std::move(rhs._callstack);
            _type = rhs._type;
            _function = rhs._function;
            _file = rhs._file;
            _line = rhs._line;
            _msgPrepared = rhs._msgPrepared;

            if (rhs._msgPrepared) rhs._msgPrepared = false;
        }
        return *this;
    }

    /**
     * @return The file name at which the error was generated.
     */
    const char * file() const noexcept {
        return _file;
    }

    /**
     * @return The line number at which the error was generated.
     */
    int line() const noexcept {
        return _line;
    }

    /**
     * @return The function name at which the error was generated.
     */
    const char * function() const noexcept {
        return _function;
    }

    /**
     * @return A string representation of the internal callstack snapshot.
     */
    std::string callstack() const noexcept {
        return _callstack.toString();
    }

    /**
     * @return The error message as a C-style string.
     */
    const char * what() const noexcept override { 
        if (! _msgPrepared) {
            std::stringstream s;
            s << _type;
            s << " at \"" << _function << "\" (" << _file << ":" << _line << "): ";
            s << _msg;
            s << "\nCallstack:\n" << _callstack.toString();
            const_cast<TraceableError *>(this)->_msg = strdup(s.str().c_str());
            const_cast<TraceableError *>(this)->_msgPrepared = true;
        }
        return Error::what();
    }
};


// Derived types ///////////////////////////////////////////////////////////////

/**
 * @brief An error to indicate that a search for a container element has failed.
*/
class ElementNotFoundError
:   public Error
{

public:

    /**
     * @brief Construct a new ElementNotFoundError object.
     */
    ElementNotFoundError()
    :   Error("Element not found")
    { }

    /**
     * @brief Construct a new ElementNotFoundError object.
     * 
     * @param msg The error message.
     */
    ElementNotFoundError(const char *msg)
    :   Error(msg)
    { }
};

/**
 * @brief An error to indicate that an argument is outside of some expected
 * range.
*/
class OutOfRangeError
:   public Error
{

public:

    /**
     * @brief Construct a new OutOfRangeError object.
     */
    OutOfRangeError()
    :   Error("Out of range")
    { }

    /**
     * @brief Construct a new OutOfRangeError object.
     * 
     * @param msg The error message.
     */
    OutOfRangeError(const char *msg)
    :   Error(msg)
    { }
};

/**
 * @brief An error to indicate that a timeout was reached.
*/
class TimeoutError
:   public Error
{

public:

    /**
     * @brief Construct a new TimeoutError object.
     */
    TimeoutError()
    :   Error("Timeout reached")
    { }

    /**
     * @brief Construct a new TimeoutError object.
     * 
     * @param msg The error message.
     */
    TimeoutError(const char *msg)
    :   Error(msg)
    { }
};

/**
 * @brief An error to indicate that an operation is unsupported.
*/
class UnsupportedError
:   public Error
{

public:

    /**
     * @brief Construct a new UnsupportedError object.
     */
    UnsupportedError()
    :   Error("Unsupported operation")
    { }

    /**
     * @brief Construct a new UnsupportedError object.
     * 
     * @param msg The error message.
     */
    UnsupportedError(const char *msg)
    :   Error(msg)
    { }
};

/**
 * @brief An error to indicate that a string parse failed.
 */
class StringParseError
:   public Error
{

public:

    /**
     * @brief Construct a new StringParseError object.
     */
    StringParseError()
    :   Error("Failed to parse string")
    { }

    /**
     * @brief Construct a new StringParseError object.
     * 
     * @param msg The error message.
     */
    StringParseError(const char *msg)
    :   Error(msg)
    { }
};

/**
 * @brief An error to indicate that an invalid argument or combination of
 * arguments was given.
*/
class InvalidArgument
:   public DynamicMessageError {
public:

    /**
     * @brief Construct a new InvalidArgument object.
     * 
     * @param msg One or more messages to be concatenated.
     */
    template <typename ...Msg>
    InvalidArgument(const Msg &...msg)
    :   DynamicMessageError(msg...)
    {
    }
};

/**
 * @brief A run-time error as a TraceableError.
*/
#define RuntimeError(msg) TraceableError("Runtime error", msg, __PRETTY_FUNCTION__, __FILE__, __LINE__)

/**
 * @brief A run-time error as a TraceableError based on the current (or given)
 * errno code. The error message is automatically generated based on the error
 * code.
*/
#define ErrnoRuntimeError(...) TraceableError("Runtime error", spl::core::__strerror(__VA_ARGS__), __PRETTY_FUNCTION__, __FILE__, __LINE__)

#define __CustomMessageErrnoRuntimeError_1(msg) TraceableError(msg, spl::core::__strerror(), __PRETTY_FUNCTION__, __FILE__, __LINE__)
#define __CustomMessageErrnoRuntimeError_2(msg, err) TraceableError(msg, spl::core::__strerror(err), __PRETTY_FUNCTION__, __FILE__, __LINE__)
#define __CustomMessageErrnoRuntimeError(_1, _2, NAME, ...) NAME

/**
 * @brief A run-time error as a TraceableError based on the current (or given)
 * errno code. The error message is a combination of the given error message
 * and an automatically generated message based on the error code.
*/
#define CustomMessageErrnoRuntimeError(...) __CustomMessageErrnoRuntimeError(__VA_ARGS__, __CustomMessageErrnoRuntimeError_2, __CustomMessageErrnoRuntimeError_1, UNUSED)(__VA_ARGS__)

}
