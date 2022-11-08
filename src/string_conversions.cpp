/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <string_conversions.h>

using namespace spl;

const uint8 StringConversions::_digitToVal[] = {
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 9, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
    __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL, __NVAL,
};

const char StringConversions::_valToDigit[] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z'
};

thread_local char StringConversions::_numBuf[__NUMBER_BUFFER_SIZE];
thread_local char StringConversions::_expBuf[__NUMBER_BUFFER_SIZE];
