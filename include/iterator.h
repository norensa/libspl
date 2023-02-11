/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <iterator>

#include <iterator_view.h>

namespace spl {

/**
 * @brief Base class for all forward iterators.
 * 
 * @tparam IteratorType The iterator type implementing this base class.
 * @tparam T The type of objects this iterator accesses.
 * @tparam _Distance The Distance type between any iterator positions.
 * @tparam _Pointer A pointer type to the type of objects this iterator
 * accesses.
 * @tparam _Reference A refernce type of the type of objects this iterator
 * accesses.
 */
template <
    typename IteratorType,
    typename T,
    typename _Distance = ptrdiff_t,
    typename _Pointer = T*,
    typename _Reference = T&
>
class ForwardIterator
:   public std::iterator<
        std::forward_iterator_tag,
        T,
        _Distance,
        _Pointer,
        _Reference
    >
{
private:

    const IteratorType & iterator() const {
        return *static_cast<const IteratorType *>(this);
    }

    IteratorType & iterator() {
        return *static_cast<IteratorType *>(this);
    }

    using base = typename std::iterator<std::forward_iterator_tag, T, _Distance, _Pointer, _Reference>;

public:

    using value_type = typename base::value_type;
    using difference_type = typename base::difference_type;
    using reference = typename base::reference;
    using pointer = typename base::pointer;

    /**
     * @brief Functionally maps the output of this iterator.
     * 
     * @param[in] f The mapping functor.
     * @return A new ForwardIteratorView object that maps the output of this
     * iterator.
     */
    template <typename F>
    ForwardIteratorView<IteratorType, F> map(F f) const {
        return ForwardIteratorView<IteratorType, F>(iterator(), f);
    }
};

/**
 * @brief Base class for all bidirectional iterators.
 * 
 * @tparam IteratorType The iterator type implementing this base class.
 * @tparam T The type of objects this iterator accesses.
 * @tparam _Distance The Distance type between any iterator positions.
 * @tparam _Pointer A pointer type to the type of objects this iterator
 * accesses.
 * @tparam _Reference A refernce type of the type of objects this iterator
 * accesses.
 */
template <
    typename IteratorType,
    typename T,
    typename _Distance = ptrdiff_t,
    typename _Pointer = T*,
    typename _Reference = T&
>
class BidirectionalIterator
:   public std::iterator<
        std::bidirectional_iterator_tag,
        T,
        _Distance,
        _Pointer,
        _Reference
    >
{
private:

    const IteratorType & iterator() const {
        return *static_cast<const IteratorType *>(this);
    }

    IteratorType & iterator() {
        return *static_cast<IteratorType *>(this);
    }

    using base = typename std::iterator<std::bidirectional_iterator_tag, T, _Distance, _Pointer, _Reference>;

public:

    using value_type = typename base::value_type;
    using difference_type = typename base::difference_type;
    using reference = typename base::reference;
    using pointer = typename base::pointer;

    /**
     * @brief Functionally maps the output of this iterator.
     * 
     * @param[in] f The mapping functor.
     * @return A new BidirectionalIteratorView object that maps the output of
     * this iterator.
     */
    template <typename F>
    BidirectionalIteratorView<IteratorType, F> map(F f) const {
        return BidirectionalIteratorView<IteratorType, F>(iterator(), f);
    }
};

}   // namespace spl
