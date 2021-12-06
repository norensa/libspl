/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <serialization.h>

// std::vector /////////////////////////////////////////////////////////////////
#include <vector>

template <typename T>
spl::OutputStreamSerializer & operator<<(
    spl::OutputStreamSerializer &serializer,
    const std::vector<T> &vector
) {
    serializer << vector.size();
    for (const auto &x : vector) serializer << x;
    return serializer;
}

template <typename T>
spl::OutputRandomAccessSerializer & operator<<(
    spl::OutputRandomAccessSerializer &serializer,
    const std::vector<T> &vector
) {
    operator<<(
        static_cast<spl::OutputStreamSerializer &>(serializer),
        vector
    );
    return serializer;
}

template <typename T>
spl::InputStreamSerializer & operator>>(
    spl::InputStreamSerializer &serializer,
    std::vector<T> &vector
) {
    size_t size;
    serializer >> size;
    vector.resize(size);
    for (size_t i = 0; i < size; ++i) serializer >> vector[i];
    return serializer;
}

template <typename T>
spl::InputRandomAccessSerializer & operator>>(
    spl::InputRandomAccessSerializer &serializer,
    std::vector<T> &vector
) {
    operator>>(
        static_cast<spl::InputStreamSerializer &>(serializer),
        vector
    );
    return serializer;
}

// std::string /////////////////////////////////////////////////////////////////
#include <string>

spl::OutputStreamSerializer & operator<<(
    spl::OutputStreamSerializer &serializer,
    const std::string &string
) {
    size_t size = string.size();
    serializer << size;
    serializer.put(string.data(), size);
    return serializer;
}

spl::OutputRandomAccessSerializer & operator<<(
    spl::OutputRandomAccessSerializer &serializer,
    const std::string &string
) {
    operator<<(
        static_cast<spl::OutputStreamSerializer &>(serializer),
        string
    );
    return serializer;
}

spl::InputStreamSerializer & operator>>(
    spl::InputStreamSerializer &serializer,
    std::string &string
) {
    size_t size;
    serializer >> size;
    string.resize(size);
    serializer.get(string.data(), size);
    return serializer;
}

spl::InputRandomAccessSerializer & operator>>(
    spl::InputRandomAccessSerializer &serializer,
    std::string &string
) {
    operator>>(
        static_cast<spl::InputStreamSerializer &>(serializer),
        string
    );
    return serializer;
}
