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
        assert(false);
    }
    catch (...) {
        assert(false);
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
        assert(false);
    }
    catch (...) {
        assert(false);
    }
});

unit("exception", "ErrnoRuntimeError")
.body([] {
    try {
        throw ErrnoRuntimeError(EINVAL);
    }
    catch (Error &e) { }
    catch (std::exception &e) {
        assert(false);
    }
    catch (...) {
        assert(false);
    }

    try {
        errno = EINVAL;
        throw ErrnoRuntimeError();
    }
    catch (Error &e) { }
    catch (std::exception &e) {
        assert(false);
    }
    catch (...) {
        assert(false);
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
        assert(false);
    }
    catch (std::exception &e) {
        assert(false);
    }
    catch (...) {
        assert(false);
    }
});
