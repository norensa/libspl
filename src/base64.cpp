/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
 * 
 * Contains refactored code from the following author(s):
 *     Copyright (c) 2016 tomykaira.
 *     Original code by tomykaira published under the MIT license.
 *     https://gist.github.com/tomykaira/f0fd86b6c73063283afe550bc5d77594
*/

#include <base64.h>
#include <exception.h>
#include <cstdlib>
#include <stdint.h>

using namespace spl;

static const char sEncodingTable[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

static const unsigned char kDecodingTable[] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

char * Base64::encode(const void *data, size_t inputLength, size_t &outputLength) {

    char *in = (char *) data;
    outputLength = 4 * ((inputLength + 2) / 3);
    size_t i;
    char *out = (char *) malloc(outputLength);
    char *p = out;

    for (i = 0; i < inputLength - 2; i += 3) {
        *p++ = sEncodingTable[(in[i] >> 2) & 0x3F];
        *p++ = sEncodingTable[((in[i] & 0x3) << 4) | ((int) (in[i + 1] & 0xF0) >> 4)];
        *p++ = sEncodingTable[((in[i + 1] & 0xF) << 2) | ((int) (in[i + 2] & 0xC0) >> 6)];
        *p++ = sEncodingTable[in[i + 2] & 0x3F];
    }
    if (i < inputLength) {
        *p++ = sEncodingTable[(in[i] >> 2) & 0x3F];
        if (i == (inputLength - 1)) {
            *p++ = sEncodingTable[((in[i] & 0x3) << 4)];
            *p++ = '=';
        }
        else {
            *p++ = sEncodingTable[((in[i] & 0x3) << 4) | ((int) (in[i + 1] & 0xF0) >> 4)];
            *p++ = sEncodingTable[((in[i + 1] & 0xF) << 2)];
        }
        *p++ = '=';
    }

    *p++ = '\0';
    return out;
}

void * Base64::decode(const char *data, size_t inputLength, size_t &outputLength) {


    if (inputLength % 4 != 0) {
        throw InvalidArgument("Input data size is not a multiple of 4");
    }

    outputLength = inputLength / 4 * 3;
    char *out = (char *) malloc(outputLength);
    if (data[inputLength - 1] == '=') outputLength--;
    if (data[inputLength - 2] == '=') outputLength--;

    for (size_t i = 0, j = 0; i < inputLength;) {
        uint32_t a = data[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(data[i++])];
        uint32_t b = data[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(data[i++])];
        uint32_t c = data[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(data[i++])];
        uint32_t d = data[i] == '=' ? 0 & i++ : kDecodingTable[static_cast<int>(data[i++])];

        uint32_t triple = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

        out[j++] = (triple >> 2 * 8) & 0xFF;
        out[j++] = (triple >> 1 * 8) & 0xFF;
        out[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return out;
}
