/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <string_conversions.h>
#include <numeric_types.h>

module("string-conversions")
.dependsOn({
    "exception"
});

#define PERFORMANCE_ITERATIONS 1000000
#define PERFORMANCE_MARGIN (0.99)

using namespace spl;

unit("string-conversions", "parse<uint8>")
.body([] {
    assert(( StringConversions::parse<uint8, 10>(  "0") == (uint8)   0 ));
    assert(( StringConversions::parse<uint8, 10>("123") == (uint8) 123 ));
    assert(( StringConversions::parse<uint8, 10>("255") == (uint8) 255 ));

    assert(( StringConversions::parse<uint8, 10>(  "0") == StringConversions::parse<uint8>(  "0") ));
    assert(( StringConversions::parse<uint8, 10>("123") == StringConversions::parse<uint8>("123") ));
    assert(( StringConversions::parse<uint8, 10>("255") == StringConversions::parse<uint8>("255") ));

    assert(( StringConversions::parse<uint8, 16>( "0") == (uint8)    0 ));
    assert(( StringConversions::parse<uint8, 16>("ab") == (uint8) 0xab ));
    assert(( StringConversions::parse<uint8, 16>("AB") == (uint8) 0xab ));

    assert(( StringConversions::parse<uint8, 8>("12") == (uint8) 012 ));
    assert(( StringConversions::parse<uint8, 8>("77") == (uint8) 077 ));
});

unit("string-conversions", "parse<int8>")
.body([] {
    assert(( StringConversions::parse<int8, 10>(   "0") == (int8)    0 ));
    assert(( StringConversions::parse<int8, 10>(  "-0") == (int8)    0 ));
    assert(( StringConversions::parse<int8, 10>( "123") == (int8)  123 ));
    assert(( StringConversions::parse<int8, 10>( "127") == (int8)  127 ));
    assert(( StringConversions::parse<int8, 10>("-128") == (int8) -128 ));

    assert(( StringConversions::parse<int8, 10>(   "0") == StringConversions::parse<int8>(   "0") ));
    assert(( StringConversions::parse<int8, 10>( "123") == StringConversions::parse<int8>( "123") ));
    assert(( StringConversions::parse<int8, 10>( "127") == StringConversions::parse<int8>( "127") ));
    assert(( StringConversions::parse<int8, 10>("-128") == StringConversions::parse<int8>("-128") ));

    assert(( StringConversions::parse<int8, 16>(  "0") == (int8)     0 ));
    assert(( StringConversions::parse<int8, 16>( "4f") == (int8)  0x4f ));
    assert(( StringConversions::parse<int8, 16>( "7f") == (int8)  0x7f ));
    assert(( StringConversions::parse<int8, 16>("-80") == (int8) -0x80 ));

    assert(( StringConversions::parse<int8, 8>("12") == (int8) 012 ));
    assert(( StringConversions::parse<int8, 8>("77") == (int8) 077 ));
});

unit("string-conversions", "parse<float64>")
.body([] {
    assert(( StringConversions::parse<float64, 10>(       "0") == (float64)        0 ));
    assert(( StringConversions::parse<float64, 10>(      "-0") == (float64)        0 ));
    assert(( StringConversions::parse<float64, 10>(     "123") == (float64)      123 ));
    assert(( StringConversions::parse<float64, 10>(     "127") == (float64)      127 ));
    assert(( StringConversions::parse<float64, 10>(    "-128") == (float64)     -128 ));
    assert(( StringConversions::parse<float64, 10>(    "-128") == (float64)     -128 ));
    assert(( StringConversions::parse<float64, 10>(     "1.2") == (float64)      1.2 ));
    assert(( StringConversions::parse<float64, 10>(  "1.2e15") == (float64)   1.2e15 ));
    assert(( StringConversions::parse<float64, 10>( "1.23e-7") == (float64)  1.23e-7 ));

    assert(( StringConversions::parse<float64>(       "0") == (float64)        0 ));
    assert(( StringConversions::parse<float64>(      "-0") == (float64)        0 ));
    assert(( StringConversions::parse<float64>(     "123") == (float64)      123 ));
    assert(( StringConversions::parse<float64>(     "127") == (float64)      127 ));
    assert(( StringConversions::parse<float64>(    "-128") == (float64)     -128 ));
    assert(( StringConversions::parse<float64>(    "-128") == (float64)     -128 ));
    assert(( StringConversions::parse<float64>(     "1.2") == (float64)      1.2 ));
    assert(( StringConversions::parse<float64>(  "1.2e15") == (float64)   1.2e15 ));
    assert(( StringConversions::parse<float64>( "1.23e-7") == (float64)  1.23e-7 ));
});

perf("string-conversions", "parse<int64>(p)")
.performanceMarginAsBaselineRatio(PERFORMANCE_MARGIN)
.body([] {
    int64 l;
    for (uint64 i = 0; i < PERFORMANCE_ITERATIONS; ++i) {
        l = StringConversions::parse_unprotected<int64>("12345");
    }
    printf("%ld", l);
})
.baseline([] {
    long l;
    for (uint64 i = 0; i < PERFORMANCE_ITERATIONS; ++i) {
        l = atol("12345");
    }
    printf("%ld", l);
});

perf("string-conversions", "parse<uint64>(p)")
.performanceMarginAsBaselineRatio(PERFORMANCE_MARGIN)
.body([] {
    uint64 l;
    for (uint64 i = 0; i < PERFORMANCE_ITERATIONS; ++i) {
        l = StringConversions::parse_unprotected<uint64>("12345");
    }
    printf("%ld", l);
})
.baseline([] {
    long l;
    for (uint64 i = 0; i < PERFORMANCE_ITERATIONS; ++i) {
        l = atol("12345");
    }
    printf("%ld", l);
});

perf("string-conversions", "parse<float64>(p)")
.performanceMarginAsBaselineRatio(PERFORMANCE_MARGIN)
.body([] {
    float64 l;
    for (uint64 i = 0; i < PERFORMANCE_ITERATIONS; ++i) {
        l = StringConversions::parse_unprotected<float64>("12345.6789");
    }
    printf("%lf", l);
})
.baseline([] {
    float64 l;
    for (uint64 i = 0; i < PERFORMANCE_ITERATIONS; ++i) {
        l = atof("12345.6789");
    }
    printf("%lf", l);
});

unit("string-conversions", "toStr(uint8)")
.body([] {
    assert(( strcmp(StringConversions::toStr<10>((uint8)   0),   "0") == 0 ));
    assert(( strcmp(StringConversions::toStr<10>((uint8) 123), "123") == 0 ));
    assert(( strcmp(StringConversions::toStr<10>((uint8) 255), "255") == 0 ));

    assert(( strcmp(StringConversions::toStr((uint8)   0),   "0") == 0 ));
    assert(( strcmp(StringConversions::toStr((uint8) 123), "123") == 0 ));
    assert(( strcmp(StringConversions::toStr((uint8) 255), "255") == 0 ));

    assert(( strcmp(StringConversions::toStr<16>((uint8)    0),  "0") == 0 ));
    assert(( strcmp(StringConversions::toStr<16>((uint8) 0xab), "AB") == 0 ));

    assert(( strcmp(StringConversions::toStr<8>((uint8) 012), "12") == 0 ));
    assert(( strcmp(StringConversions::toStr<8>((uint8) 077), "77") == 0 ));
});

unit("string-conversions", "toStr(int8)")
.body([] {
    assert(( strcmp(StringConversions::toStr<10>((int8)    0),    "0") == 0 ));
    assert(( strcmp(StringConversions::toStr<10>((int8)  123),  "123") == 0 ));
    assert(( strcmp(StringConversions::toStr<10>((int8)  127),  "127") == 0 ));
    assert(( strcmp(StringConversions::toStr<10>((int8) -128), "-128") == 0 ));

    assert(( strcmp(StringConversions::toStr((int8)    0),    "0") == 0 ));
    assert(( strcmp(StringConversions::toStr((int8)  123),  "123") == 0 ));
    assert(( strcmp(StringConversions::toStr((int8)  127),  "127") == 0 ));
    assert(( strcmp(StringConversions::toStr((int8) -128), "-128") == 0 ));

    assert(( strcmp(StringConversions::toStr<16>((int8)     0),   "0") == 0 ));
    assert(( strcmp(StringConversions::toStr<16>((int8)  0x4f),  "4F") == 0 ));
    assert(( strcmp(StringConversions::toStr<16>((int8)  0x7f),  "7F") == 0 ));
    assert(( strcmp(StringConversions::toStr<16>((int8) -0x80), "-80") == 0 ));

    assert(( strcmp(StringConversions::toStr<8>((int8) 012), "12") == 0 ));
    assert(( strcmp(StringConversions::toStr<8>((int8) 077), "77") == 0 ));
});

unit("string-conversions", "toStr<float64>")
.body([] {
    assert(( strcmp(StringConversions::toStr<6, 5, 10>((float64)        0            ),       "0"               ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5, 10>((float64)      123            ),     "123"               ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5, 10>((float64)      127            ),     "127"               ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5, 10>((float64)     -128            ),    "-128"               ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5, 10>((float64)        1.1          ),       "1.10000"         ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5, 10>((float64)   999999            ),  "999999"               ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5, 10>((float64)   999999.999999999  ),  "999999.99999"         ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5, 10>((float64)  1000000            ),       "1e6"             ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5, 10>((float64) 11000000            ),       "1.10000e7"       ) == 0 ));

    assert(( strcmp(StringConversions::toStr<6, 5>((float64)        0            ),       "0"               ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5>((float64)      123            ),     "123"               ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5>((float64)      127            ),     "127"               ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5>((float64)     -128            ),    "-128"               ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5>((float64)        1.1          ),       "1.10000"         ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5>((float64)   999999            ),  "999999"               ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5>((float64)   999999.999999999  ),  "999999.99999"         ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5>((float64)  1000000            ),       "1e6"             ) == 0 ));
    assert(( strcmp(StringConversions::toStr<6, 5>((float64) 11000000            ),       "1.10000e7"       ) == 0 ));
});

perf("string-conversions", "toStr<int64>(p)")
.performanceMarginAsBaselineRatio(PERFORMANCE_MARGIN)
.body([] {
    for (int64 i = 0; i < PERFORMANCE_ITERATIONS; ++i) {
        StringConversions::toStr(i);
    }
})
.baseline([] {
    char buf[20];
    for (int64 i = 0; i < PERFORMANCE_ITERATIONS; ++i) {
        snprintf(buf, 20, "%ld", i);
    }
});

perf("string-conversions", "toStr<uint64>(p)")
.performanceMarginAsBaselineRatio(PERFORMANCE_MARGIN)
.body([] {
    for (uint64 i = 0; i < PERFORMANCE_ITERATIONS; ++i) {
        StringConversions::toStr(i);
    }
})
.baseline([] {
    char buf[20];
    for (uint64 i = 0; i < PERFORMANCE_ITERATIONS; ++i) {
        snprintf(buf, 20, "%lu", i);
    }
});

perf("string-conversions", "toStr<float64>(p)")
.performanceMarginAsBaselineRatio(PERFORMANCE_MARGIN)
.body([] {
    for (uint64 i = 0; i < PERFORMANCE_ITERATIONS; ++i) {
        StringConversions::toStr((float64) i);
    }
})
.baseline([] {
    char buf[20];
    for (uint64 i = 0; i < PERFORMANCE_ITERATIONS; ++i) {
        snprintf(buf, 20, "%lf", (float64) i);
    }
});
