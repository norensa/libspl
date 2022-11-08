/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <callstack.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>    // for __cxa_demangle
#include <sstream>

using namespace spl;

CallStack CallStack::trace(int skip) {
    ++skip;
    void **stack = (void **) malloc((_MAX_STACK_FRAMES + skip) * sizeof(void *));
    int nFrames = backtrace(stack, _MAX_STACK_FRAMES + skip);

    return { nFrames, stack, skip };
}

std::string CallStack::toString() const noexcept {
    std::stringstream s;
    char buf[1024];

    char **symbols = backtrace_symbols(_stack, _len);

    for (int i = _skip; i < _len; i++) {
        Dl_info info;
        if (dladdr(_stack[i], &info)) {
            char *demangled = NULL;
            int status;
            demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
            snprintf(
                buf, sizeof(buf), "%-3d  %p  %s + %p%s",
                _len - i - 1, _stack[i],
                status == 0 ? demangled : info.dli_sname,
                (void *) ((char *)_stack[i] - (char *)info.dli_saddr),
                (i == _len - 1) ? "" : "\n"
            );
            free(demangled);
        }
        else {
            snprintf(
                buf, sizeof(buf), "%-3d  %p  %s%s",
                _len - i - 1, _stack[i],
                symbols[i],
                (i == _len - 1) ? "" : "\n"
            );
        }
        s << buf;
    }
    free(symbols);
    if (_len == CallStack::_MAX_STACK_FRAMES + _skip) s << "\n[truncated]";

    return s.str();
}
