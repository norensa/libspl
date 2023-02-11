/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <traits.h>
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace spl {

    /**
     * @brief Combines one or more hash codes.
     * 
     * @param[in] hash A hash code.
     * @return The combined hash code.
     */
    inline size_t hash_combine(size_t hash) {
        return hash;
    }

    /**
     * @brief Combines one or more hash codes.
     * 
     * @param[in] hash1,hash2,... Hash codes.
     * @return The combined hash code.
     */
    template <typename... U>
    inline size_t hash_combine(size_t hash1, size_t hash2, U... hash) {
        hash1 ^= hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2);
        return hash_combine(hash1, hash...);
    }

    /**
     * @brief Calculates the hash code of a stream of bytes.
     * 
     * @param[in] data Pointer to data.
     * @param[in] len Length in bytes.
     * @return The calculated hash code.
     */
    inline size_t hash(const void *data, size_t len) {
        const char *s = (const char *) data;
        const char *end = s + len;
        // http://www.cse.yorku.ca/~oz/hash.html
        size_t h = 5381;
        int c;
        while (s != end) {
            c = *s++;
            h = ((h << 5) + h) + c;
        }
        return h;
    }

    /**
     * @brief Calculates the hash code of a null-terminated string.
     * 
     * @param[in] str Pointer to a null-terminate string.
     * @return The calculated hash code.
     */
    inline size_t hash(const char *str) {
        // http://www.cse.yorku.ca/~oz/hash.html
        size_t h = 5381;
        int c;
        while (*str != '\0') {
            c = *str++;
            h = ((h << 5) + h) + c;
        }
        return h;
    }

    /**
     * @brief Squeezes a long hash word into 32 bits.
     * 
     * @param h A size_t hash word.
     * @return The shortened 32-bit hash.
     */
    inline uint32_t short_hash(size_t h) {
        static_assert(
            sizeof(size_t) == 4 || sizeof(size_t) == 8,
            "Unsupported size_t size"
        );

        if (sizeof(size_t) == 4) {
            return h;
        }
        else {
            return static_cast<uint32_t>(h) ^ static_cast<uint32_t>(h >> 32);
        }
    }

    /**
     * @brief The default functor for calculating the hash codes of objects. If
     * the object supports the Hashable trait, the `hash() const` function is
     * used. Otherwise, std::hash is used to evaluate a hash code.
     */
    template <typename T>
    struct Hash {
        template <
            typename X = T,
            typename std::enable_if<std::is_base_of<Hashable, X>::value, int>::type = 0
        >
        size_t operator()(const T &t) const {
            return t.hash();
        }

        template <
            typename X = T,
            typename std::enable_if<! std::is_base_of<Hashable, X>::value, int>::type = 0
        >
        size_t operator()(const T &t) const {
            return std::hash<T>{}(t);
        }
    };

}   // namespace spl
