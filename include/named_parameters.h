/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <serialization.h>
#include <hash_map.h>
#include <string>
#include <std_serialization.h>
#include <exception.h>
#include <sstream>
#include <json.h>

namespace spl {

/**
 * @brief A class for serializable named parameters.
 */
class NamedParameters
:   public Serializable
{

private:

    struct Param : Serializable {
        std::string defaultValue;
        std::string value;

        Param() = default;

        Param(const std::string &defaultValue)
        :   defaultValue(defaultValue)
        { }

        void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
            serializer << defaultValue << value;
        }

        void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
            serializer >> defaultValue >> value;
        }
    };

    HashMap<std::string, Param> _param;

public:

    NamedParameters() = default;

    NamedParameters(const NamedParameters &) = default;

    NamedParameters(NamedParameters &&) = default;

    ~NamedParameters() = default;

    NamedParameters & operator=(const NamedParameters &) = default;

    NamedParameters & operator=(NamedParameters &&) = default;

    void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
        serializer << _param;
    }

    void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
        serializer >> _param;
    }

    /**
     * @brief Checks whether a parameter with the given key exists.
     * 
     * @param key The key to check.
     * @return true if the key exists, false otherwise.
     */
    bool contains(const std::string &key) const {
        return _param.contains(key);
    }

    /**
     * @brief Adds a named parameter of type T to the stored parameters.
     * 
     * @tparam T Type of the parameter. The supported primitive types are
     * std::string, bool, and numeric types. In addition lists (List<U>) and
     * dictionaries (HashMap<std::string, U>), where U is a primitive type or
     * list/dictionary, are also supported.
     * @param key Parameter name.
     * @param defaultValue The default value of the parameter.
     * @return A reference to this object for chaining.
     */
    template <typename T>
    NamedParameters & addParameter(const std::string &key, const T &defaultValue) {
        _param.put(key, JSON::encode(defaultValue));
        return *this;
    }

    /**
     * @brief Adds a named parameter of type T to the stored parameters.
     * 
     * @tparam T Type of the parameter. The supported primitive types are
     * std::string, bool, and numeric types. In addition lists (List<U>) and
     * dictionaries (HashMap<std::string, U>), where U is a primitive type or
     * list/dictionary, are also supported.
     * @param key Parameter name.
     * @param defaultValue The default value of the parameter.
     * @return A reference to this object for chaining.
     */
    template <typename T>
    NamedParameters & addParameter(std::string &&key, const T &defaultValue) {
        _param.put(std::move(key), JSON::encode(defaultValue));
        return *this;
    }

    /**
     * @brief Sets the value of a named parameter. If the named parameter does
     * not exist, an ElementNotFoundError is thrown.
     * 
     * @tparam T Type of the parameter. The supported primitive types are
     * std::string, bool, and numeric types. In addition lists (List<U>) and
     * dictionaries (HashMap<std::string, U>), where U is a primitive type or
     * list/dictionary, are also supported.
     * @param key Parameter name.
     * @param value The value of the parameter.
     * @throws ElementNotFoundError if the named parameter does not exist.
     * @return A reference to this object for chaining.
     */
    template <typename T>
    NamedParameters & set(const std::string &key, const T &value) {
        if (! _param.contains(key)) throw ElementNotFoundError();

        _param[key].value = JSON::encode(value);

        return *this;
    }

    /**
     * @brief Resets the value of a named parameter. If the named parameter does
     * not exist, an ElementNotFoundError is thrown.
     * 
     * @param key Parameter name.
     * @throws ElementNotFoundError if the named parameter does not exist.
     * @return A reference to this object for chaining.
     */
    NamedParameters & reset(const std::string &key) {
        if (! _param.contains(key)) throw ElementNotFoundError();

        _param[key].value.clear();

        return *this;
    }

    /**
     * @brief Gets the value of a named parameter. If the parameter value is not
     * set, the default value is returned. If the named parameter does not
     * exist, an ElementNotFoundError is thrown. Throws InvalidArgument if the
     * parameter cannot be extracted as type T.
     * 
     * @tparam T Type of the parameter. The supported primitive types are
     * std::string, bool, and numeric types. In addition lists (List<U>) and
     * dictionaries (HashMap<std::string, U>), where U is a primitive type or
     * list/dictionary, are also supported.
     * @param key Parameter name.
     * @throws ElementNotFoundError if the named parameter does not exist.
     * @throws InvalidArgument if the parameter is not compatible with type T.
     * @return The parameter value.
     */
    template <typename T>
    T get(const std::string &key) const {
        if (! _param.contains(key)) throw ElementNotFoundError();

        auto &p = _param[key];

        try {
            return JSON::decode<T>(p.value.empty() ? p.defaultValue : p.value);
        }
        catch (const JSONDecodeError &e) {
            throw InvalidArgument(
                "Error extracting conf key '", key, "' as '", typeid(T).name(), '\''
            );
        }
    }

    /**
     * @param key Parameter name.
     * @throws ElementNotFoundError if the named parameter does not exist.
     * @return True if the parameter has a value other than the default value,
     * false otherwise.
     */
    bool isSet(const std::string &key) const {
        if (! _param.contains(key)) throw ElementNotFoundError();
        return ! _param[key].value.empty();
    }
};

} // namespace spl 
