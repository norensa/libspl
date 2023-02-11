/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <functional>
#include <typeinfo>
#include <hash.h>

namespace spl {

/**
 * @brief Repository for class factory functions.
*/
class Factory {

private:

    static void _put(size_t hashCode, void *factory);

    static void * _get(size_t hashCode);

    static size_t _hash(size_t code) {
        return code;
    }

    static size_t _hash(const std::type_info &type) {
        return type.hash_code();
    }

    template <typename T>
    static size_t _hash() {
        return typeid(T).hash_code();
    }

    template <typename ...T>
    static size_t _hash(const std::type_info &type) {
        return hash_combine(_hash(type), _hash<T>()...);
    }

    template <typename ...T>
    static size_t _hash(size_t code) {
        return hash_combine(_hash(code), _hash<T>()...);
    }

public:

    /**
     * @brief Registers the given class type and corresponding factory function.
     * 
     * @param[in] type Typeid of the class.
     * @param[in] factory Factory function of the indicated class type.
     */
    static void registerFactory(
        const std::type_info &type,
        const std::function<void *()> &factory
    ) {
        _put(_hash(type), new std::function<void *()>(factory));
    }

    /**
     * @brief Registers the given class type and corresponding factory function.
     * 
     * @tparam Args Factory function argument types.
     * 
     * @param[in] type Typeid of the class.
     * @param[in] factory Factory function of the indicated class type.
     */
    template <typename ...Args>
    static void registerFactory(
        const std::type_info &type,
        const std::function<void *(Args...)> &factory
    ) {
        _put(_hash<Args...>(type), new std::function<void *(Args...)>(factory));
    }

    /**
     * @brief Creates a new instance of the indicated object type.
     * 
     * @tparam T Return type (only affects return pointer type).
     * @param[in] hashCode Hash code of the typeid of the actual desired class.
     * @return A new instance of the desired type.
     */
    template <typename T>
    static T * createObject(size_t hashCode) {
        return static_cast<T *>(
            static_cast<std::function<void *()> *>(
                _get(_hash(hashCode))
            )->operator()()
        );
    }

    /**
     * @brief Creates a new instance of the indicated object type.
     * 
     * @tparam T Return type (only affects return pointer type).
     * @param[in] hashCode Hash code of the typeid of the actual desired class.
     * @param[in] args Factory function arguments.
     * @return A new instance of the desired type.
     */
    template <typename T, typename ...Args>
    static T * createObject(size_t hashCode, Args ...args) {
        return static_cast<T *>(
            static_cast<std::function<void *(Args...)> *>(
                _get(_hash<Args...>(hashCode))
            )->operator()(args...)
        );
    }

    /**
     * @brief Creates a new instance of the indicated object type.
     * 
     * @tparam T Return type (only affects return pointer type).
     * @param[in] type [optional] Typeid of the actual desired class.
     * @return A new instance of the desired type.
     */
    template <typename T>
    static T * createObject(const std::type_info &type = typeid(T)) {
        return createObject<T>(_hash(type));
    }

    /**
     * @brief Creates a new instance of the indicated object type.
     * 
     * @tparam T Return type (only affects return pointer type).
     * @param[in] type Typeid of the actual desired class.
     * @param[in] args Factory function arguments.
     * @return A new instance of the desired type.
     */
    template <typename T, typename ...Args>
    static T * createObject(const std::type_info &type, Args ...args) {
        return createObject<T, Args...>(_hash<Args...>(type), args...);
    }
};

/**
 * @brief Registers a constructor as a factory method for the type T.
 * 
 * @tparam T The type for which a factory method will be created.
 * @tparam Args The constructor arguments.
 */
template <typename T, typename ...Args>
class WithFactory {
private:
    static struct _Init {    
        // to prevent the compiler from optimizing-out this entire struct
        bool initialized = false;
        _Init() {
            Factory::registerFactory<Args...>(
                typeid(T),
                std::function<void *(Args...)>([] (Args... args) {
                    return new T(args...);
                })
            );
        }
    } __init;

public:
    WithFactory() {
        __init.initialized = true;
    }
};

template <typename T, typename ...Args>
typename WithFactory<T, Args...>::_Init WithFactory<T, Args...>::__init;

}   // namespace spl
