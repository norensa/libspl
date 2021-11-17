/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <functional>
#include <typeinfo>
#include <hash_map.h>
#include <exception.h>

namespace spl {

/**
 * @brief Repository for class factory functions.
*/
class Factory {

private:

    static HashMap<size_t, void *> & _factory();

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
        if (_factory().contains(type.hash_code())) {
            throw RuntimeError("Duplicate object hash codes detected");
        }
        _factory().put(type.hash_code(), new std::function(factory));
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
        if (_factory().contains(type.hash_code())) {
            throw RuntimeError("Duplicate object hash codes detected");
        }
        _factory().put(type.hash_code(), new std::function(factory));
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
        if (! _factory().contains(hashCode)) {
            throw InvalidArgument("No registered factory for this object type");
        }
        return static_cast<T *>(
            static_cast<std::function<void *()> *>(
                _factory()[hashCode]
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
        if (! _factory().contains(hashCode)) {
            throw InvalidArgument("No registered factory for this object type");
        }
        return static_cast<T *>(
            static_cast<std::function<void *(Args...)> *>(
                _factory()[hashCode]
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
        return createObject<T>(type.hash_code());
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
        return createObject<T, Args...>(type.hash_code(), args...);
    }
};

/**
 * @brief Registers the default no-argument constructor as a factory method for
 * the type T.
 * 
 * @tparam T The type for which a factory method will be created.
 */
template <typename T>
class WithDefaultFactory {
private:

    static inline class _Init {
        friend class WithDefaultFactory;

        // to prevent the compiler from optimizing-out this entire class
        bool initialized = false;
        _Init() {
            Factory::registerFactory(typeid(T), [] { return new T(); });
        }
    } __init;

public:

    WithDefaultFactory() {
        __init.initialized = true;
    }
};

}   // namespace spl
