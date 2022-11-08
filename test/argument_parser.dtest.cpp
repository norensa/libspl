/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <argument_parser.h>

module("argument-parser")
.dependsOn({
    "string-conversions",
    "hash-map"
});

using namespace spl;

unit("argument-parser", "store-bool")
.body([] {
    ArgumentParser parser;
    bool val = false;

    parser.add(
        Argument("--opt")
        .numParams(1)
        .action(Argument::store(val))
    );

    const char *args1[] = {
        "",
        "--opt",
        "true"
    };
    parser.parse(sizeof(args1) / sizeof(args1[0]), args1);
    assert(val == true);

    const char *args2[] = {
        "",
        "--opt",
        "false"
    };
    parser.parse(sizeof(args2) / sizeof(args2[0]), args2);
    assert(val == false);
});

unit("argument-parser", "store-bool-err")
.expect(Status::FAIL)
.body([] {
    ArgumentParser parser;
    bool val = false;

    parser.add(
        Argument("--opt")
        .numParams(1)
        .action(Argument::store(val))
    );

    const char *args1[] = {
        "",
        "--opt",
        "whatever"
    };
    parser.parse(sizeof(args1) / sizeof(args1[0]), args1);
});

unit("argument-parser", "set-bool")
.body([] {
    ArgumentParser parser;
    bool val = false;

    parser.add(
        Argument("--opt")
        .action(Argument::set(val, true))
    );

    const char *args1[] = {
        "",
        "--opt"
    };
    parser.parse(sizeof(args1) / sizeof(args1[0]), args1);
    assert(val == true);
});

unit("argument-parser", "store-string")
.body([] {
    ArgumentParser parser;
    std::string val;

    parser.add(
        Argument("--opt")
        .numParams(1)
        .action(Argument::store(val))
    );

    const char *args1[] = {
        "",
        "--opt",
        "mystr"
    };
    parser.parse(sizeof(args1) / sizeof(args1[0]), args1);
    assert(val == "mystr");

    const char *args2[] = {
        "",
        "--opt",
        "mystr2"
    };
    parser.parse(sizeof(args2) / sizeof(args2[0]), args2);
    assert(val == "mystr2");
});

unit("argument-parser", "set-string")
.body([] {
    ArgumentParser parser;
    std::string val;

    parser.add(
        Argument("--opt")
        .action(Argument::set(val, "hello"))
    );

    const char *args1[] = {
        "",
        "--opt"
    };
    parser.parse(sizeof(args1) / sizeof(args1[0]), args1);
    assert(val == "hello");
});

unit("argument-parser", "store-numeric")
.body([] {
    ArgumentParser parser;
    int val = 0;

    parser.add(
        Argument("--opt")
        .numParams(1)
        .action(Argument::store(val))
    );

    const char *args1[] = {
        "",
        "--opt",
        "123"
    };
    parser.parse(sizeof(args1) / sizeof(args1[0]), args1);
    assert(val == 123);

    const char *args2[] = {
        "",
        "--opt",
        "-15"
    };
    parser.parse(sizeof(args2) / sizeof(args2[0]), args2);
    assert(val == -15);
});

unit("argument-parser", "set-numeric")
.body([] {
    ArgumentParser parser;
    int val = 0;

    parser.add(
        Argument("--opt")
        .action(Argument::set(val, 4))
    );

    const char *args1[] = {
        "",
        "--opt"
    };
    parser.parse(sizeof(args1) / sizeof(args1[0]), args1);
    assert(val == 4);
});

unit("argument-parser", "store-char-ptr")
.body([] {
    ArgumentParser parser;
    const char *val = nullptr;

    parser.add(
        Argument("--opt")
        .numParams(1)
        .action(Argument::store(val))
    );

    const char *args1[] = {
        "",
        "--opt",
        "something"
    };
    parser.parse(sizeof(args1) / sizeof(args1[0]), args1);
    assert(val == args1[2]);

    const char *args2[] = {
        "",
        "--opt",
        "something else"
    };
    parser.parse(sizeof(args2) / sizeof(args2[0]), args2);
    assert(val == args2[2]);
});
