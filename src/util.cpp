#include <util.h>
#include <exception.h>

#define ABS(n) (n < 0 ? -n : n) 

using namespace spl;

const char * spl::core::__timepoint_to_str(
    uint64_t timepoint,
    uint64_t periodDenom,
    struct tm *tm,
    unsigned int precision
) {

    static thread_local char str[64];

    if (tm == nullptr) throw RuntimeError("localtime() error");

    if (precision > 0) {
        snprintf(
            str,
            sizeof(str),
            "%04d-%02d-%02d %02d:%02d:%0*.*f %+03ld%02ld",
            tm->tm_year + 1900,
            tm->tm_mon + 1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            precision + 3,
            precision,
            (double) tm->tm_sec + (double) (timepoint % periodDenom) / (double) periodDenom,
            tm->tm_gmtoff / 3600,
            ABS(tm->tm_gmtoff % 60)
        );
    }
    else {
        snprintf(
            str,
            sizeof(str),
            "%04d-%02d-%02d %02d:%02d:%02d %+03ld%02ld",
            tm->tm_year + 1900,
            tm->tm_mon + 1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec,
            tm->tm_gmtoff / 3600,
            ABS(tm->tm_gmtoff % 60)
        );
    }

    return str;

}