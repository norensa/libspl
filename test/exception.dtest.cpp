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

static void unsuppoted_function() {
    throw FunctionUnsupportedError();
}
unit("exception", "FunctionUnsupportedError")
.body([] {
    try {
        unsuppoted_function();
    }
    catch (UnsupportedError &e) {
        std::cout << e.what();
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
