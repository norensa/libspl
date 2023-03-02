/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <sstream>
#include <chrono>
#include <time.h>

namespace spl {

namespace core {

inline std::stringstream __make_str() {
    return std::stringstream();
}

template <typename T>
std::stringstream __make_str(const T &t) {
    std::stringstream s;
    s << t;
    return s;
}

template <typename T, typename ...U>
std::stringstream __make_str(const T &t, const U &...u) {
    std::stringstream s;
    s << t;
    s << __make_str(u...).rdbuf();
    return s;
}

const char * __timepoint_to_str(
    uint64_t timepoint,
    uint64_t periodDenom,
    struct tm *tm,
    unsigned int precision
);

} // namespace core 

template <typename ...T>
std::string make_str(const T &...t) {
    return core::__make_str(t...).str();
}

#ifndef LIBSPL_EMBEDDED

template <
    unsigned int precision = 3,
    typename T
>
const char * timepoint_to_str(const T &timepoint) {
    time_t t = T::clock::to_time_t(timepoint);

    return core::__timepoint_to_str(
        (uint64_t) timepoint.time_since_epoch().count(),
        T::period::den,
        localtime(&t),
        precision
    );
}

template <
    unsigned int precision = 3,
    typename T
>
const char * timepoint_to_str_utc(const T &timepoint) {
    time_t t = T::clock::to_time_t(timepoint);

    return core::__timepoint_to_str(
        (uint64_t) timepoint.time_since_epoch().count(),
        T::period::den,
        gmtime(&t),
        precision
    );

}

#endif  // LIBSPL_EMBEDDED

}
