/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <cstdlib>

namespace spl {

/**
 * @brief Manages base64 encoding and decoding.
*/
class Base64 {
public:

    /**
     * @brief Encodes a given data block to a base64 C-style string.
     * 
     * @param[in] data Const pointer to a data block.
     * @param[in] inputLength Length of the input data block.
     * @param[out] outputLength Length of the output data.
     * @return Pointer to a malloc'ed C-style string containing the base64
     * representation of the given data block.
     */
    static char * encode(const void *data, size_t inputLength, size_t &outputLength);

    /**
     * @brief Decodes a base64 C-style string.
     * 
     * @param[in] data Const pointer to a base64 C-style string.
     * @param[in] inputLength Length of the input string
     * @param[out] outputLength Length of the output data.
     * @return Pointer to a malloc'ed data block.
     */
    static void * decode(const char *data, size_t inputLength, size_t &outputLength);
};

}
