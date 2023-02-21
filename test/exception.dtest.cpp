/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>

#include <exception.h>

using namespace spl;

unit("exception", "Error")
.body([] {
    const char m[] = "test message";
    try {
        throw Error(m);
    }
    catch (Error &e) {
        assert(strcmp(e.what(), m) == 0);
    }
    catch (std::exception &e) {
        fail("Unexpected exception type");
    }
    catch (...) {
        fail("Unknown object thrown");
    }
});

unit("exception", "RuntimeError")
.body([] {
    const char m[] = "test message";
    try {
        throw RuntimeError(m);
    }
    catch (Error &e) { }
    catch (std::exception &e) {
        fail("Unexpected exception type");
    }
    catch (...) {
        fail("Unknown object thrown");
    }
});

unit("exception", "ErrnoRuntimeError")
.body([] {
    try {
        throw ErrnoRuntimeError(EINVAL);
    }
    catch (Error &e) { }
    catch (std::exception &e) {
        fail("Unexpected exception type");
    }
    catch (...) {
        fail("Unknown object thrown");
    }

    try {
        errno = EINVAL;
        throw ErrnoRuntimeError();
    }
    catch (Error &e) { }
    catch (std::exception &e) {
        fail("Unexpected exception type");
    }
    catch (...) {
        fail("Unknown object thrown");
    }
});

unit("exception", "InvalidArgument")
.body([] {
    try {
        throw InvalidArgument("This ", "is ", "an ", "error ", "5");
    }
    catch (InvalidArgument &e) {
        assert(strcmp(e.what(), "This is an error 5") == 0);
    }
    catch (Error &e) {
        fail("Unexpected exception type");
    }
    catch (std::exception &e) {
        fail("Unexpected exception type");
    }
    catch (...) {
        fail("Unknown object thrown");
    }
});

static void unsupported_function() {
    throw FunctionUnsupportedError();
}
template <typename T>
static void unsupported_function_template() {
    throw FunctionUnsupportedError();
}
class unsupported_class_function {
public:

    static void function() {
        throw FunctionUnsupportedError();
    }

    template <typename T>
    static void function_template() {
        throw FunctionUnsupportedError();
    }
};
unit("exception", "FunctionUnsupportedError-1")
.body([] {
    try {
        unsupported_function();
    }
    catch (UnsupportedError &e) {
        std::cout << e.what() << std::endl;
    }
    catch (Error &e) {
        fail("Unexpected exception type");
    }
    catch (std::exception &e) {
        fail("Unexpected exception type");
    }
    catch (...) {
        fail("Unknown object thrown");
    }
});

unit("exception", "FunctionUnsupportedError-2")
.body([] {
    try {
        unsupported_function_template<int>();
    }
    catch (UnsupportedError &e) {
        std::cout << e.what() << std::endl;
    }
    catch (Error &e) {
        fail("Unexpected exception type");
    }
    catch (std::exception &e) {
        fail("Unexpected exception type");
    }
    catch (...) {
        fail("Unknown object thrown");
    }
});

unit("exception", "FunctionUnsupportedError-3")
.body([] {
    try {
        unsupported_class_function().function();
    }
    catch (UnsupportedError &e) {
        std::cout << e.what() << std::endl;
    }
    catch (Error &e) {
        fail("Unexpected exception type");
    }
    catch (std::exception &e) {
        fail("Unexpected exception type");
    }
    catch (...) {
        fail("Unknown object thrown");
    }
});

unit("exception", "FunctionUnsupportedError-4")
.body([] {
    try {
        unsupported_class_function().function_template<int>();
    }
    catch (UnsupportedError &e) {
        std::cout << e.what() << std::endl;
    }
    catch (Error &e) {
        fail("Unexpected exception type");
    }
    catch (std::exception &e) {
        fail("Unexpected exception type");
    }
    catch (...) {
        fail("Unknown object thrown");
    }
});
