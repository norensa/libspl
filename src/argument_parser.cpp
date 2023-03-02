/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <argument_parser.h>

using namespace spl;

#ifndef LIBSPL_EMBEDDED

ArgumentParser::ArgumentParser(const std::initializer_list<Argument> &arguments)
:   _args(arguments.size())
{
    for (const auto &a : arguments) _args.put(a._argument, a);
}

ArgumentParser & ArgumentParser::parse(int argc, const char * const *argv) {
    // ignore argv[0]
    --argc;
    ++argv;

    while (argc > 0) {
        if (! _args.contains(argv[0])) {
            throw DynamicMessageError("Unknown argument '", argv[0], "' encountered");
        }

        const auto &arg = _args[argv[0]];

        --argc;
        ++argv;

        if (argc < arg._numParams) {
            throw DynamicMessageError("Insufficient parameters supplied to '", arg._argument, "'");
        }

        if (arg._action && ! arg._action(argv)) {
            throw DynamicMessageError("Error during parsing argument '", arg._argument, "'");
        }

        argc -= arg._numParams;
        argv += arg._numParams;
    }

    return *this;
}

#endif
