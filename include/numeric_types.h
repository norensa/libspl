/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <limits.h>
#include <float.h>

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;

typedef char int8;
typedef short int16;
typedef int int32;
typedef long int64;

typedef float float32;
typedef double float64;
typedef long double float128;

template <typename T>
struct numeric_type_info {
    static constexpr T min = 0;
    static constexpr T max = 0;
    static constexpr bool isSigned = false;
    static constexpr bool isIntegral = false;
    static constexpr bool isFloating = false;
};

template <>
struct numeric_type_info<uint8> {
    static constexpr uint8 min = 0;
    static constexpr uint8 max = UCHAR_MAX;
    static constexpr bool isSigned = false;
    static constexpr bool isIntegral = true;
    static constexpr bool isFloating = false;
};

template <>
struct numeric_type_info<uint16> {
    static constexpr uint16 min = 0;
    static constexpr uint16 max = USHRT_MAX;
    static constexpr bool isSigned = false;
    static constexpr bool isIntegral = true;
    static constexpr bool isFloating = false;
};

template <>
struct numeric_type_info<uint32> {
    static constexpr uint32 min = 0;
    static constexpr uint32 max = UINT_MAX;
    static constexpr bool isSigned = false;
    static constexpr bool isIntegral = true;
    static constexpr bool isFloating = false;
};

template <>
struct numeric_type_info<uint64> {
    static constexpr uint64 min = 0;
    static constexpr uint64 max = ULONG_MAX;
    static constexpr bool isSigned = false;
    static constexpr bool isIntegral = true;
    static constexpr bool isFloating = false;
};

template <>
struct numeric_type_info<int8> {
    static constexpr int8 min = CHAR_MIN;
    static constexpr int8 max = CHAR_MAX;
    static constexpr bool isSigned = true;
    static constexpr bool isIntegral = true;
    static constexpr bool isFloating = false;
};

template <>
struct numeric_type_info<int16> {
    static constexpr int16 min = SHRT_MIN;
    static constexpr int16 max = SHRT_MAX;
    static constexpr bool isSigned = true;
    static constexpr bool isIntegral = true;
    static constexpr bool isFloating = false;
};

template <>
struct numeric_type_info<int32> {
    static constexpr int32 min = INT_MIN;
    static constexpr int32 max = INT_MAX;
    static constexpr bool isSigned = true;
    static constexpr bool isIntegral = true;
    static constexpr bool isFloating = false;
};

template <>
struct numeric_type_info<int64> {
    static constexpr int64 min = LONG_MIN;
    static constexpr int64 max = LONG_MAX;
    static constexpr bool isSigned = true;
    static constexpr bool isIntegral = true;
    static constexpr bool isFloating = false;
};

template <>
struct numeric_type_info<float32> {
    static constexpr float32 min = FLT_MIN;
    static constexpr float32 max = FLT_MAX;
    static constexpr bool isSigned = true;
    static constexpr bool isIntegral = false;
    static constexpr bool isFloating = true;
};

template <>
struct numeric_type_info<float64> {
    static constexpr float64 min = DBL_MIN;
    static constexpr float64 max = DBL_MAX;
    static constexpr bool isSigned = true;
    static constexpr bool isIntegral = false;
    static constexpr bool isFloating = true;
};

template <>
struct numeric_type_info<float128> {
    static constexpr float128 min = LDBL_MIN;
    static constexpr float128 max = LDBL_MAX;
    static constexpr bool isSigned = true;
    static constexpr bool isIntegral = false;
    static constexpr bool isFloating = true;
};
