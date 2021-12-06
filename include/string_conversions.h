#pragma once

#include <cmath>
#include <string>
#include <numeric_types.h>
#include <exception.h>
#include <type_traits>

namespace spl {

struct StringNotNumeric
:   StringParseError
{
    StringNotNumeric()
    :   StringParseError("String contains non-numeric characters")
    { }
};

class StringConversions {

private:

    static constexpr size_t __NUMBER_BUFFER_SIZE = 100;
    static constexpr size_t __INTEGER_START = 50;
    static constexpr size_t __INTEGER_END = 51;
    static constexpr size_t __POINT = 50;
    static constexpr size_t __FRACTION_START = 51;

    static constexpr uint8 __NVALD = (uint8) -1;

    static constexpr uint8 _digitToVal[] = {
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30, 31, 32,
        33, 34, 35, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30, 31, 32,
        33, 34, 35, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
        __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD, __NVALD,
    };

    static constexpr char _valToDigit[] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
        'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
        'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
        'W', 'X', 'Y', 'Z'
    };

    static thread_local char _numBuf[__NUMBER_BUFFER_SIZE];
    static thread_local char _expBuf[__NUMBER_BUFFER_SIZE];

public:

    template <typename T, int base = 10>
    static T str_to_unsigned_int_unprotected(const char *str) {
        T x = 0;
        while (*str != '\0') {
            x = (x * (T) base) + (T) _digitToVal[(size_t) *str];
            ++str;
        }
        return x;
    }

    template <typename T, int base = 10>
    static T str_to_unsigned_int(const char *str) {
        T x = 0;
        while (*str != '\0') {
            if (_digitToVal[(size_t) *str] == __NVALD) throw StringNotNumeric();
            x = (x * (T) base) + (T) _digitToVal[(size_t) *str];
            ++str;
        }
        return x;
    }

    template <typename T, int base = 10>
    static const char * unsigned_int_to_str(T val) {
        char *str = _numBuf + __INTEGER_START;
        *str = '\0';
        do {
            --str;
            *str = _valToDigit[(size_t) (val % base)];
            val /= base;
        } while (val);
        return str;
    }

    template <typename T, int base = 10>
    static T str_to_int_unprotected(const char *str) {
        T x = 0;
        bool neg = false;
        if (*str == '-') {
            neg = true;
            ++str;
        }
        while (*str != '\0') {
            x = (x * (T) base) + (T) _digitToVal[(size_t) *str];
            ++str;
        }
        if (neg) x = -x;
        return x;
    }

    template <typename T, int base = 10>
    static T str_to_int(const char *str) {
        T x = 0;
        bool neg = false;
        if (*str == '-') {
            neg = true;
            ++str;
        }
        while (*str != '\0') {
            if (_digitToVal[(size_t) *str] == __NVALD) throw StringNotNumeric();
            x = (x * (T) base) + (T) _digitToVal[(size_t) *str];
            ++str;
        }
        if (neg) x = -x;
        return x;
    }

    template <typename T, int base = 10>
    static const char * int_to_str(T val) {
        bool neg = false;
        char *str = _numBuf +__INTEGER_START;

        if (val >= 0) val = -val;
        else neg = true;

        *str = '\0';
        do {
            --str;
            *str = _valToDigit[(size_t) (-(val % base))];
            val /= base;
        } while (val);

        if (neg) {
            --str;
            *str = '-';
        }

        return str;
    }

    template <typename T, int base = 10>
    static T str_to_float_unprotected(const char *str) {
        T x = 0;
        bool neg = false;
        if (*str == '-') {
            neg = true;
            ++str;
        }
        while (_digitToVal[(size_t) *str] != __NVALD && _digitToVal[(size_t) *str] < base) {
            x = (x * (T) base) + _digitToVal[(size_t) *str];
            ++str;
        }
        if (*str == '.') {
            ++str;
            T f = (T) 1 / (T) base;
            while (_digitToVal[(size_t) *str] != __NVALD && _digitToVal[(size_t) *str] < base) {
                x += _digitToVal[(size_t) *str] * f;
                ++str;
                f *= (T) 1 / (T) base;
            }
        }
        if (*str == 'e' || *str == 'E') {
            ++str;
            x *= pow((T) base, str_to_int_unprotected<int, base>(str));
        }
        if (neg) x = -x;
        return x;
    }

    template <typename T, int base = 10>
    static T str_to_float(const char *str) {
        T x = 0;
        bool neg = false;
        if (*str == '-') {
            neg = true;
            ++str;
        }
        while (_digitToVal[(size_t) *str] != __NVALD && _digitToVal[(size_t) *str] < base) {
            x = (x * (T) base) + _digitToVal[(size_t) *str];
            ++str;
        }
        if (*str == '.') {
            ++str;
            T f = (T) 1 / (T) base;
            while (_digitToVal[(size_t) *str] != __NVALD && _digitToVal[(size_t) *str] < base) {
                x += _digitToVal[(size_t) *str] * f;
                ++str;
                f *= (T) 1 / (T) base;
            }
        }
        if (*str == 'e' || *str == 'E') {
            ++str;
            x *= pow((T) base, str_to_int<int, base>(str));
        }
        else if (*str != '\0') {
            throw StringParseError("Unexpected characters encountered");
        }
        if (neg) x = -x;
        return x;
    }

    template <typename T, int precision = 6, int fractionPrecision = precision, int base = 10>
    static const char * float_to_str(T val) {
        constexpr T precisionLimit = pow((T) base, precision);
        constexpr T fractionPrecisionMultiplier = pow((T) base, fractionPrecision);
        constexpr T expDivisor = log2((T) base);

        int exp;        // exponent
        T norm;         // normalized fraction
        int64 i;        // integer part
        uint64 f;       // fraction part
        char *str;      // pointer to some previous conversion / final result
        size_t len;     // length of some previous conversion
        char *expStart; // pointer to exponent string start

        if (val < precisionLimit) {
            i = (int64) val;
            f = (uint64) ((val - i) * fractionPrecisionMultiplier);
            if (f > 0) {
                str = (char *) unsigned_int_to_str<uint64, base>(f);
                memcpy(_numBuf + __FRACTION_START, str, _numBuf + __INTEGER_END - str);
                str = (char *) int_to_str<int64, base>(i);
                _numBuf[__POINT] = '.';
            }
            else {
                str = (char *) int_to_str<int64, base>(i);
            }
        }
        else {
            exp = (T) log2(val) / expDivisor;
            norm = val / pow((T) base, exp);

            i = (int64) norm;
            f = (uint64) ((norm - i) * fractionPrecisionMultiplier);
            if (f > 0) {
                str = (char *) unsigned_int_to_str<uint64, base>(f);
                len = _numBuf + __INTEGER_END - str;
                memcpy(_numBuf + __FRACTION_START, str, len);

                expStart = _numBuf + __INTEGER_START + len ;
                *expStart = 'e';
                ++expStart;
                str = (char *) int_to_str<int, base>(exp);
                memcpy(expStart, str, _numBuf + __INTEGER_END - str);

                str = (char *) int_to_str<int64, base>(i);
                _numBuf[__POINT] = '.';
            }
            else {
                str = (char *) int_to_str<int, base>(exp);
                memcpy(_numBuf + __FRACTION_START, str, _numBuf + __INTEGER_END - str);

                str = (char *) int_to_str<int64, base>(i);
                _numBuf[__POINT] = 'e';
            }
        }

        return str;
    }

    template <
        typename T,
        std::enable_if_t<
            std::is_same_v<T, uint8>
            || std::is_same_v<T, uint16>
            || std::is_same_v<T, uint32>
            || std::is_same_v<T, uint64>,
            int
        > base = 10
    >
    static T parse(const char *str) {
        return str_to_unsigned_int<T, base>(str);
    }

    template <
        typename T,
        std::enable_if_t<
            std::is_same_v<T, uint8>
            || std::is_same_v<T, uint16>
            || std::is_same_v<T, uint32>
            || std::is_same_v<T, uint64>,
            int
        > base = 10
    >
    static T parse_unprotected(const char *str) {
        return str_to_unsigned_int_unprotected<T, base>(str);
    }

    template <
        typename T,
        std::enable_if_t<
            std::is_same_v<T, int8>
            || std::is_same_v<T, int16>
            || std::is_same_v<T, int32>
            || std::is_same_v<T, int64>,
            int
        > base = 10
    >
    static T parse(const char *str) {
        return str_to_int<T, base>(str);
    }

    template <
        typename T,
        std::enable_if_t<
            std::is_same_v<T, int8>
            || std::is_same_v<T, int16>
            || std::is_same_v<T, int32>
            || std::is_same_v<T, int64>,
            int
        > base = 10
    >
    static T parse_unprotected(const char *str) {
        return str_to_int_unprotected<T, base>(str);
    }

    template <
        typename T,
        std::enable_if_t<
            std::is_same_v<T, float32>
            || std::is_same_v<T, float64>
            || std::is_same_v<T, float128>,
            int
        > base = 10
    >
    static T parse(const char *str) {
        return str_to_float<T, base>(str);
    }

    template <
        typename T,
        std::enable_if_t<
            std::is_same_v<T, float32>
            || std::is_same_v<T, float64>
            || std::is_same_v<T, float128>,
            int
        > base = 10
    >
    static T parse_unprotected(const char *str) {
        return str_to_float_unprotected<T, base>(str);
    }

    template <
        typename T,
        std::enable_if_t<
            std::is_same_v<T, uint8>
            || std::is_same_v<T, uint16>
            || std::is_same_v<T, uint32>
            || std::is_same_v<T, uint64>
            || std::is_same_v<T, int8>
            || std::is_same_v<T, int16>
            || std::is_same_v<T, int32>
            || std::is_same_v<T, int64>
            || std::is_same_v<T, float32>
            || std::is_same_v<T, float64>
            || std::is_same_v<T, float128>,
            int
        > base = 10
    >
    static T parse(const std::string &str) {
        return parse<T, base>(str.c_str());
    }

    template <
        typename T,
        std::enable_if_t<
            std::is_same_v<T, uint8>
            || std::is_same_v<T, uint16>
            || std::is_same_v<T, uint32>
            || std::is_same_v<T, uint64>
            || std::is_same_v<T, int8>
            || std::is_same_v<T, int16>
            || std::is_same_v<T, int32>
            || std::is_same_v<T, int64>
            || std::is_same_v<T, float32>
            || std::is_same_v<T, float64>
            || std::is_same_v<T, float128>,
            int
        > base = 10
    >
    static T parse_unprotected(const std::string &str) {
        return parse_unprotected<T, base>(str.c_str());
    }

    template <int base = 10>
    static const char * toStr(uint8 num) {
        return unsigned_int_to_str<uint8, base>(num);
    }

    template <int base = 10>
    static const char * toStr(uint16 num) {
        return unsigned_int_to_str<uint16, base>(num);
    }

    template <int base = 10>
    static const char * toStr(uint32 num) {
        return unsigned_int_to_str<uint32, base>(num);
    }

    template <int base = 10>
    static const char * toStr(uint64 num) {
        return unsigned_int_to_str<uint64, base>(num);
    }

    template <int base = 10>
    static const char * toStr(int8 num) {
        return int_to_str<int8, base>(num);
    }

    template <int base = 10>
    static const char * toStr(int16 num) {
        return int_to_str<int16, base>(num);
    }

    template <int base = 10>
    static const char * toStr(int32 num) {
        return int_to_str<int32, base>(num);
    }

    template <int base = 10>
    static const char * toStr(int64 num) {
        return int_to_str<int64, base>(num);
    }

    template <int precision = 6, int fractionPrecision = precision, int base = 10>
    static const char * toStr(float32 num) {
        return float_to_str<float32, precision, fractionPrecision, base>(num);
    }

    template <int precision = 6, int fractionPrecision = precision, int base = 10>
    static const char * toStr(float64 num) {
        return float_to_str<float64, precision, fractionPrecision, base>(num);
    }

    template <int precision = 6, int fractionPrecision = precision, int base = 10>
    static const char * toStr(float128 num) {
        return float_to_str<float128, precision, fractionPrecision, base>(num);
    }
};

}   // namespace spl