#include <dtest.h>
#include <argumentparser.h>

module("argumentparser")
.dependsOn({
    "string-conversions",
    "hash-map"
});

using namespace spl;

unit("argumentparser", "store-bool")
.body([] {
    ArgumentParser parser;
    bool val = false;

    parser.add(
        Argument("--opt")
        .numParams(1)
        .action(Argument::store(val))
    );

    const char *args1[] = {
        "--opt",
        "true"
    };
    parser.parse(2, args1);
    assert(val == true);

    const char *args2[] = {
        "--opt",
        "false"
    };
    parser.parse(2, args2);
    assert(val == false);
});

unit("argumentparser", "store-bool-err")
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
        "--opt",
        "whatever"
    };
    parser.parse(2, args1);
});

unit("argumentparser", "set-bool")
.body([] {
    ArgumentParser parser;
    bool val = false;

    parser.add(
        Argument("--opt")
        .action(Argument::set(val, true))
    );

    const char *args1[] = {
        "--opt"
    };
    parser.parse(1, args1);
    assert(val == true);
});

unit("argumentparser", "store-string")
.body([] {
    ArgumentParser parser;
    std::string val;

    parser.add(
        Argument("--opt")
        .numParams(1)
        .action(Argument::store(val))
    );

    const char *args1[] = {
        "--opt",
        "mystr"
    };
    parser.parse(2, args1);
    assert(val == "mystr");

    const char *args2[] = {
        "--opt",
        "mystr2"
    };
    parser.parse(2, args2);
    assert(val == "mystr2");
});

unit("argumentparser", "set-string")
.body([] {
    ArgumentParser parser;
    std::string val;

    parser.add(
        Argument("--opt")
        .action(Argument::set(val, "hello"))
    );

    const char *args1[] = {
        "--opt"
    };
    parser.parse(1, args1);
    assert(val == "hello");
});

unit("argumentparser", "store-numeric")
.body([] {
    ArgumentParser parser;
    int val = 0;

    parser.add(
        Argument("--opt")
        .numParams(1)
        .action(Argument::store(val))
    );

    const char *args1[] = {
        "--opt",
        "123"
    };
    parser.parse(2, args1);
    assert(val == 123);

    const char *args2[] = {
        "--opt",
        "-15"
    };
    parser.parse(2, args2);
    assert(val == -15);
});

unit("argumentparser", "set-numeric")
.body([] {
    ArgumentParser parser;
    int val = 0;

    parser.add(
        Argument("--opt")
        .action(Argument::set(val, 4))
    );

    const char *args1[] = {
        "--opt"
    };
    parser.parse(1, args1);
    assert(val == 4);
});
