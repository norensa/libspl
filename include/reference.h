/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <memory>

namespace spl {

/**
 * @brief A smart-pointer object that provides unique ownership sematics.
 * 
 * @tparam T Type of the object.
 * @tparam Deleter A deleter function to deallocate memory. Defaults to
 * std::default_delete<T>
 */
template <
    typename T,
    typename Deleter = std::default_delete<T>
>
class Reference {

private:

    T *_obj = nullptr;
    Deleter _deleter;

public:

    /**
     * @brief Construct a new Reference object
     * 
     */
    Reference() = default;

    /**
     * @brief Construct a new Reference object
     * 
     * @param obj Pointer to the managed object.
     */
    Reference(T *obj)
    :   _obj(obj)
    { }

    Reference(const Reference &rhs) = delete;

    Reference(Reference &&rhs)
    :   _obj(rhs._obj)
    {
        rhs._obj = nullptr;
    }

    ~Reference() {
        if (_obj != nullptr) _deleter(_obj);
    }

    Reference & operator=(const Reference &) = delete;

    Reference & operator=(Reference &&rhs) {
        if (this != &rhs) {
            if (_obj != nullptr) _deleter(_obj);
            _obj = rhs._obj;
            rhs._obj = nullptr;
        }
        return *this;
    }

    /**
     * @brief Releases ownership of the object.
     * 
     * @return The pointer to the managed object.
     */
    T * release() {
        T *obj = _obj;
        _obj = nullptr;
        return obj;
    }

    /**
     * @return The pointer to the managed object.
     */
    T * get() {
        return _obj;
    }

    T * operator->() {
        return _obj;
    }

    const T * operator->() const {
        return _obj;
    }

    T & operator*() {
        return *_obj;
    }

    const T & operator*() const {
        return *_obj;
    }

    operator T & () {
        return *_obj;
    }

    operator const T & () const {
        return *_obj;
    }
};

}
