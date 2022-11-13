/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <util.h>
#include <time.h>
#include <string.h>

module("util")
.dependsOn({
    "exception"
});

using namespace spl;

unit("util", "make_str")
.body([] {

    assert(make_str() == "");

    assert(make_str("abc") == "abc");

    assert(make_str("ab", 'c') == "abc");

    assert(make_str("ab", 'c', 'd') == "abcd");

    assert(make_str("abc", "def") == "abcdef");

    assert(make_str("ab", 'c', "def") == "abcdef");

    assert(make_str(123, "abc") == "123abc");

    assert(make_str("abc", 123) == "abc123");

    assert(make_str("abc", 123, "def") == "abc123def");
});

unit("util", "timepoint_to_str")
.ignoreMemoryLeak()
.body([] {

    using clock = std::chrono::system_clock;
    using namespace std::chrono;

    struct tm tm;

    setenv("TZ", "", 1);
    tzset();

    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 1970 - 1900;
    tm.tm_mday = 1;
    tm.tm_isdst = 0;
    std::cout << timepoint_to_str_utc<3>(clock::from_time_t(mktime(&tm))) << '\n';
    assert(strcmp(
        timepoint_to_str_utc<3>(clock::from_time_t(mktime(&tm))),
        "1970-01-01 00:00:00.000 +0000"
    ) == 0);

    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 1970 - 1900;
    tm.tm_mon = 1;
    tm.tm_mday = 1;
    tm.tm_hour = 2;
    tm.tm_min = 3;
    tm.tm_sec = 4;
    std::cout << timepoint_to_str_utc<3>(clock::from_time_t(mktime(&tm))) << '\n';
    assert(strcmp(
        timepoint_to_str_utc<3>(clock::from_time_t(mktime(&tm))),
        "1970-02-01 02:03:04.000 +0000"
    ) == 0);

    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 1970 - 1900;
    tm.tm_mday = 1;
    std::cout << timepoint_to_str_utc<3>(clock::from_time_t(mktime(&tm)) + milliseconds(123)) << '\n';
    assert(strcmp(
        timepoint_to_str_utc<3>(clock::from_time_t(mktime(&tm)) + milliseconds(123)),
        "1970-01-01 00:00:00.123 +0000"
    ) == 0);
});
