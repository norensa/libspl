/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

namespace spl {

/**
 * @brief Base class for all containers with forward iteration capability. A
 * container is forward iterable if it only supports forward iterators, or if
 * it has no specific ordering.
 * 
 * @tparam ContainerType The actual implemented container type
*/
template <typename ContainerType>
class ForwardIterableContainer {
private:

    constexpr const ContainerType & container() const {
        return *static_cast<const ContainerType *>(this);
    }

    constexpr ContainerType & container() {
        return *static_cast<ContainerType *>(this);
    }

public:

    /**
     * @brief Iterates over the elements of this container applying the given
     * function.
     * 
     * @param[in] f The functor to call for every element.
     */
    template <typename F>
    const ContainerType & foreach(F f) const {
        for (const auto &x : container()) f(x);
        return container();
    }

    /**
     * @brief Iterates over the elements of this container applying the given
     * function.
     * 
     * @param[in] f The functor to call for every element.
     */
    template <typename F>
    ContainerType & foreach(F f) {
        for (auto &x : container()) f(x);
        return container();
    }

    /**
     * @brief Maps the elements of this container using the given function.
     * 
     * @tparam MappedType Type of the mapped container.
     * @param[in] mapper A mapping functor that transforms the elements.
     * @return A new container with the mapped elements.
     */
    template <typename MappedType = ContainerType, typename F>
    auto map(F mapper) const {
        return MappedType::create(
            container().begin().map(mapper),
            container().end(),
            container().size()
        );
    }

    /**
     * @brief Creates a new container of the indicated type and copies the
     * elements of this container into it.
     * 
     * @tparam T Type of the new container.
     * @return A new container with the copied elements.
     */
    template <typename T>
    auto to() const {
        return T(
            container().begin(),
            container().end(),
            container().size()
        );
    }

    /**
     * @brief Reduces the elements of this container to a single value.
     * 
     * @param[in] reducer A reducing functor that combines two elements into
     * one. The reducing function should be commutative as no order of execution
     * is guaranteed. The reducer is guaranteed to be called with some
     * accumulator as the first parameter and some new element as the second
     * parameter.
     * @return A single value after applying the reducing function to all
     * elements.
     */
    template <typename F>
    auto reduce(F reducer) const {
        using ref = decltype(*container().begin());
        using reduce_type = typename std::invoke_result_t<F, ref, ref>;

        auto it = container().begin();
        auto end = container().end();
        reduce_type res;
        if (it != end) {
            res = *it;
            while (++it != end) {
                res = reducer(res, *it);
            }
        }
        return res;
    }

    /**
     * @brief Reduces the elements of this container to a single value.
     * 
     * @param[in] initialMapper An initial mapping functor that is invoked for
     * the first element of the reduce operation.
     * @param[in] reducer A reducing functor that combines two elements into
     * one. The reducing function should be commutative as no order of execution
     * is guaranteed. The reducer is guaranteed to be called with some
     * accumulator as the first parameter and some new element as the second
     * parameter.
     * @return A single value after applying the reducing function to all
     * elements.
     */
    template <typename Init, typename F>
    auto reduce(Init initialMapper, F reducer) const {
        using ref = decltype(*container().begin());
        using reduce_type = typename std::invoke_result_t<Init, ref>;

        auto it = container().begin();
        auto end = container().end();
        reduce_type res;
        if (it != end) {
            res = initialMapper(*it);
            while (++it != end) {
                res = reducer(res, *it);
            }
        }
        return res;
    }
};

}   // namespace spl
