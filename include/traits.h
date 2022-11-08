/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

namespace spl {

/**
 * @brief A trait for hashable objects defining the public member function
 * `size_t hash() const`.
*/
struct Hashable {};

/**
 * @brief A trait for polymorphic copyable objects supporting the virtual
 * function `copy() const`.
 * 
 * @tparam T Base type of a series of copyable types.
 */
template <typename T>
struct Copyable {

    virtual T * copy() const = 0;
};

/**
 * @brief A trait for polymorphic movable objects supporting the virtual
 * function `move()`.
 * 
 * @tparam T Base type of a series of movable types.
 */
template <typename T>
struct Movable {

    virtual T * move() = 0;
};

}   // namespace spl
