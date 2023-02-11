/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <string>
#include <sstream>
#include <cstring>
#include <string_conversions.h>
#include <list.h>
#include <hash_map.h>
#include <exception.h>
#include <numeric_types.h>

namespace spl {

/**
 * @brief An error to indicate that a JSON decode failed.
 */
struct JSONDecodeError
:   StringParseError
{
    JSONDecodeError()
    :   StringParseError("Failed to decode JSON string")
    { }

    JSONDecodeError(const char *msg)
    :   StringParseError(msg)
    { }
};

/**
 * @brief Manages JSON encoding and decoding.
 */
class JSON {

private:

    static const char *_WHITE_SPACE;
    static const char *_NUM_STOP;

    static std::string _indent(const std::string &str, int spaces = 2);

    static void _skipWhitespaces(char const * &str) {
        str += strspn(str, _WHITE_SPACE);
    }

    static bool _consumeToken(char const * &str, char token) {
        _skipWhitespaces(str);
        if (*str != token) return false;
        ++str;
        return true;
    }

    static std::string _extractString(char const * &str);

    static std::string _extractToken(char const * &str, const char *stop) {
        size_t count = strcspn(str, stop);
        auto start = str;
        str += count;
        return std::string(start, count);
    }

    static void _escape(std::stringstream &s, const char *str);

    static bool _unescape(std::stringstream &s, const char *str);

public:

    /**
     * @brief Encodes a null-terminated string into its JSON representation.
     * 
     * @param val Pointer to a null-terminated string.
     * @return JSON encoding of val.
     */
    static std::string encode(const char *val) {
        std::stringstream s;
        s << '"';
        _escape(s, val);
        s << '"';
        return s.str();
    }

    /**
     * @brief Encodes a string into its JSON representation.
     * 
     * @param val A string.
     * @return JSON encoding of val.
     */
    static std::string encode(const std::string &val) {
        return encode(val.c_str());
    }

    /**
     * @brief Encodes a boolean into its JSON representation.
     * 
     * @param val A boolean value.
     * @return JSON encoding of val.
     */
    static std::string encode(const bool &val) {
        return val ? "true" : "false";
    }

    /**
     * @brief Encodes a numeric value into its JSON representation.
     * 
     * @param val A number.
     * @return JSON encoding of val.
     */
    template <
        typename T,
        typename std::enable_if<
            std::is_same<T, uint8>::value
            || std::is_same<T, uint16>::value
            || std::is_same<T, uint32>::value
            || std::is_same<T, uint64>::value
            || std::is_same<T, int8>::value
            || std::is_same<T, int16>::value
            || std::is_same<T, int32>::value
            || std::is_same<T, int64>::value
            || std::is_same<T, float32>::value
            || std::is_same<T, float64>::value
            || std::is_same<T, float128>::value,
            int
        >::type = 0
    >
    static std::string encode(const T &val) {
        return StringConversions::toStr(val);
    }

    /**
     * @brief Encodes a list into its JSON representation.
     * 
     * @tparam T Type of the list items.
     * @param list The list to encode.
     * @return JSON encoding of the list.
     */
    template <typename T>
    static std::string encode(const List<T> &list) {
        if (list.empty()) {
            return "[ ]";
        }
        else {
            std::stringstream s;
            s << '[';
            for (auto it = list.begin(), end = list.end(); it != end; ) {
                s << "\n  ";
                s << _indent(encode(*it));

                ++it;
                if (it != end) s << ',';
            }
            s << "\n]";
            return s.str();
        }
    }

    /**
     * @brief Encodes a dictionary into its JSON representation.
     * 
     * @tparam T Type of the dictionary items.
     * @param map The dictionary to encode.
     * @return JSON encoding of the dictionary.
     */
    template <typename T>
    static std::string encode(const HashMap<std::string, T> &map) {
        if (map.empty()) {
            return "{ }";
        }
        else {
            std::stringstream s;
            s << '{';
            for (auto it = map.begin(), end = map.end(); it != end; ) {
                s << "\n  ";
                s << encode(it->k);
                s << ": ";
                s << _indent(encode(it->v));

                ++it;
                if (it != end) s << ',';
            }
            s << "\n}";
            return s.str();
        }
    }

private:

    static bool _decode(char const * &str, std::string &val) {
        if (! _consumeToken(str, '"')) return false;
        std::stringstream s;
        if (! _unescape(s, _extractString(str).c_str())) return false;
        if (! _consumeToken(str, '"')) return false;
        val = s.str();
        return true;
    }

    static bool _decode(char const * &str, bool &val) {
        if (strncasecmp(str, "true", 4) == 0) {
            str += 4;
            val = true;
        }
        else if (strncasecmp(str, "false", 5) == 0) {
            str += 5;
            val = false;
        }
        else {
            return false;
        }

        return true;
    }

    template <
        typename T,
        typename std::enable_if<
            std::is_same<T, uint8>::value
            || std::is_same<T, uint16>::value
            || std::is_same<T, uint32>::value
            || std::is_same<T, uint64>::value
            || std::is_same<T, int8>::value
            || std::is_same<T, int16>::value
            || std::is_same<T, int32>::value
            || std::is_same<T, int64>::value
            || std::is_same<T, float32>::value
            || std::is_same<T, float64>::value
            || std::is_same<T, float128>::value,
            int
        >::type = 0
    >
    static bool _decode(char const * &str, T &val) {
        try {
            std::string t = _extractToken(str, _NUM_STOP);
            if (t.empty()) return false;
            val = StringConversions::parse<T>(t);
            return true;
        }
        catch (const StringParseError &e) {
            return false;
        }
    }

    template <typename T>
    static bool _decode(char const * &str, List<T> &list) {
        if (! _consumeToken(str, '[')) return false;

        _skipWhitespaces(str);
        if (_consumeToken(str, ']')) return true;

        do {
            _skipWhitespaces(str);
            T val;
            if (! _decode(str, val)) return false;
            list.append(std::move(val));
        } while (_consumeToken(str, ','));

        if (! _consumeToken(str, ']')) return false;

        return true;
    }

    template <typename T>
    static bool _decode(char const * &str, HashMap<std::string, T> &map) {
        if (! _consumeToken(str, '{')) return false;

        _skipWhitespaces(str);
        if (_consumeToken(str, '}')) return true;

        do {
            _skipWhitespaces(str);

            std::string key;
            if (! _decode(str, key)) return false;

            if (! _consumeToken(str, ':')) return false;
            _skipWhitespaces(str);

            T val;
            if (! _decode(str, val)) return false;

            map.put(std::move(key), std::move(val));
        } while (_consumeToken(str, ','));

        if (! _consumeToken(str, '}')) return false;

        return true;
    }

public:

    /**
     * @brief Decodes a JSON string into an object of type T. If the parsing
     * fails, a JSONDecodeError is thrown.
     * 
     * @tparam T Type of the object to decode.
     * @param str A null-terminated JSON string.
     * @throws JSONDecodeError if the parsing fails.
     * @return The parsed value.
     */
    template <typename T>
    static T decode(const char *str) {
        T retval;
        if (! _decode(str, retval) || *str != '\0') {
            throw JSONDecodeError();
        }
        return retval;
    }

    /**
     * @brief Decodes a JSON string into an object of type T. If the parsing
     * fails, a JSONDecodeError is thrown.
     * 
     * @tparam T Type of the object to decode.
     * @param str A JSON string.
     * @throws JSONDecodeError if the parsing fails.
     * @return The parsed value.
     */
    template <typename T>
    static T decode(const std::string &str) {
        return decode<T>(str.c_str());
    }
};

}   // namespace spl
