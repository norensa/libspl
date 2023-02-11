/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <typeinfo>
#include <string>

namespace spl {

/**
 * @brief An enhanced type info class that is copy-constructible and
 * copy-assignable. This class is constructible from and compatible with
 * std::type_info
 */
class TypeInfo {

private:

    size_t _hashCode;
    std::string _name;

public:

    /**
     * @brief Construct a new Type Info object
     */
    TypeInfo()
    :   _hashCode(0),
        _name("")
    { }

    /**
     * @brief Construct a new Type Info object
     * 
     * @param type A std::type_info object
     */
    TypeInfo(const std::type_info &type)
    :   _hashCode(type.hash_code()),
        _name(type.name())
    { }

    /**
     * @return A unique implementation-specific hash code for this type.
     */
    size_t hashCode() const {
        return _hashCode;
    }

    /**
     * @return A unique implementation-specific name string for this type.
     */
    const char * name() const {
        return _name.c_str();
    }

    bool operator==(const TypeInfo &rhs) const {
        return _hashCode == rhs._hashCode && _name == rhs._name;
    }

    bool operator==(const std::type_info &rhs) const {
        return _hashCode == rhs.hash_code() && _name == rhs.name();
    }

    bool operator!=(const TypeInfo &rhs) const {
        return ! operator==(rhs);
    }

    bool operator!=(const std::type_info &rhs) const {
        return ! operator==(rhs);
    }

    bool operator<(const TypeInfo &rhs) const {
        return _name < rhs._name;
    }

    bool operator<(const std::type_info &rhs) const {
        return _name < rhs.name();
    }

    bool operator<=(const TypeInfo &rhs) const {
        return _name <= rhs._name;
    }

    bool operator<=(const std::type_info &rhs) const {
        return _name <= rhs.name();
    }

    bool operator>(const TypeInfo &rhs) const {
        return _name > rhs._name;
    }

    bool operator>(const std::type_info &rhs) const {
        return _name > rhs.name();
    }

    bool operator>=(const TypeInfo &rhs) const {
        return _name >= rhs._name;
    }

    bool operator>=(const std::type_info &rhs) const {
        return _name >= rhs.name();
    }

    friend bool operator==(const std::type_info &lhs, const TypeInfo &rhs) {
        return rhs == lhs;
    }

    friend bool operator!=(const std::type_info &lhs, const TypeInfo &rhs) {
        return rhs != lhs;
    }

    friend bool operator<(const std::type_info &lhs, const TypeInfo &rhs) {
        return rhs > lhs;
    }

    friend bool operator<=(const std::type_info &lhs, const TypeInfo &rhs) {
        return rhs >= lhs;
    }

    friend bool operator>(const std::type_info &lhs, const TypeInfo &rhs) {
        return rhs < lhs;
    }

    friend bool operator>=(const std::type_info &lhs, const TypeInfo &rhs) {
        return rhs <= lhs;
    }

};

}
