/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <iterator>
#include <type_traits>

namespace spl {

/**
 * @brief Provides a functional view of a ForwardIterator.
 * 
 * @tparam IteratorType The actual ForwardIterator type.
 * @tparam ViewFunction The view functor to apply.
 */
template <typename IteratorType, typename ViewFunction>
class ForwardIteratorView
:   public std::iterator<
        std::forward_iterator_tag,
        typename std::invoke_result_t<ViewFunction, typename IteratorType::reference>,
        typename IteratorType::difference_type,
        void,
        typename std::invoke_result_t<ViewFunction, typename IteratorType::reference>
    >
{
private:
    IteratorType _it;
    ViewFunction _f;

public:

    using value_type = typename std::invoke_result_t<ViewFunction, typename IteratorType::reference>;
    using difference_type = typename IteratorType::difference_type;
    using reference = value_type;

    /**
     * @brief Construct a new ForwardIteratorView object.
     * 
     * @param[in] it A ForwardIterator object.
     * @param[in] f A functor to map the iterator output.
     */
    ForwardIteratorView(const IteratorType &it, const ViewFunction &f)
    :   _it(it),
        _f(f)
    { }

    ForwardIteratorView(const ForwardIteratorView &) = default;

    ForwardIteratorView(ForwardIteratorView &&) = default;

    ~ForwardIteratorView() = default;

    ForwardIteratorView & operator=(const ForwardIteratorView &) = default;

    ForwardIteratorView & operator=(ForwardIteratorView &&) = default;

    bool operator==(const ForwardIteratorView &rhs) const {
        return _it == rhs._it;
    }
    bool operator==(const IteratorType &rhs) const {
        return _it == rhs;
    }
    friend bool operator==(const IteratorType &lhs, const ForwardIteratorView &rhs) {
        return lhs == rhs._it;
    }

    bool operator!=(const ForwardIteratorView &rhs) const {
        return ! operator==(rhs);
    }
    bool operator!=(const IteratorType &rhs) const {
        return ! operator==(rhs);
    }
    friend bool operator!=(const IteratorType &lhs, const ForwardIteratorView &rhs) {
        return ! operator==(lhs, rhs);
    }

    value_type operator*() {
        return _f(_it.operator*());
    }

    ForwardIteratorView & operator++() {
        _it.operator++();
        return *this;
    }

    ForwardIteratorView operator++(int) {
        ForwardIteratorView current = *this;
        _it.operator++();
        return current;
    }

    /**
     * @brief Functionally maps the output of this iterator.
     * 
     * @param[in] f The mapping functor.
     * @return A new ForwardIteratorView object that maps the output of this
     * iterator.
     */
    template <typename F>
    ForwardIteratorView<ForwardIteratorView<IteratorType, ViewFunction>, F> map(F f) const {
        return ForwardIteratorView<ForwardIteratorView<IteratorType, ViewFunction>, F>(*this, f);
    }
};

/**
 * @brief Provides a functional view of a BidirectionalIterator.
 * 
 * @tparam IteratorType The actual BidirectionalIterator type.
 * @tparam ViewFunction The view functor to apply.
 */
template <typename IteratorType, typename ViewFunction>
class BidirectionalIteratorView
:   public std::iterator<
        std::bidirectional_iterator_tag,
        typename std::invoke_result_t<ViewFunction, typename IteratorType::reference>,
        typename IteratorType::difference_type,
        void,
        typename std::invoke_result_t<ViewFunction, typename IteratorType::reference>
    >
{
private:
    IteratorType _it;
    ViewFunction _f;

public:

    using value_type = typename std::invoke_result_t<ViewFunction, typename IteratorType::reference>;
    using difference_type = typename IteratorType::difference_type;
    using reference = value_type;

    /**
     * @brief Construct a new BidirectionalIteratorView object.
     * 
     * @param[in] it A BidirectionalIterator object.
     * @param[in] f A functor to map the iterator output.
     */
    BidirectionalIteratorView(const IteratorType &it, const ViewFunction &f)
    :   _it(it),
        _f(f)
    { }

    BidirectionalIteratorView(const BidirectionalIteratorView &) = default;

    BidirectionalIteratorView(BidirectionalIteratorView &&) = default;

    ~BidirectionalIteratorView() = default;

    BidirectionalIteratorView & operator=(const BidirectionalIteratorView &) = default;

    BidirectionalIteratorView & operator=(BidirectionalIteratorView &&) = default;

    bool operator==(const BidirectionalIteratorView &rhs) const {
        return _it == rhs._it;
    }
    bool operator==(const IteratorType &rhs) const {
        return _it == rhs;
    }
    friend bool operator==(const IteratorType &lhs, const BidirectionalIteratorView &rhs) {
        return lhs == rhs._it;
    }

    bool operator!=(const BidirectionalIteratorView &rhs) const {
        return ! operator==(rhs);
    }
    bool operator!=(const IteratorType &rhs) const {
        return ! operator==(rhs);
    }
    friend bool operator!=(const IteratorType &lhs, const BidirectionalIteratorView &rhs) {
        return ! operator==(lhs, rhs);
    }

    value_type operator*() {
        return _f(_it.operator*());
    }

    BidirectionalIteratorView & operator++() {
        _it.operator++();
        return *this;
    }

    BidirectionalIteratorView operator++(int) {
        BidirectionalIteratorView current = *this;
        _it.operator++();
        return current;
    }

    BidirectionalIteratorView & operator--() {
        _it.operator--();
        return *this;
    }

    BidirectionalIteratorView operator--(int) {
        BidirectionalIteratorView current = *this;
        _it.operator--();
        return current;
    }

    /**
     * @brief Functionally maps the output of this iterator.
     * 
     * @param[in] f The mapping functor.
     * @return A new BidirectionalIteratorView object that maps the output of
     * this iterator.
     */
    template <typename F>
    BidirectionalIteratorView<BidirectionalIteratorView<IteratorType, ViewFunction>, F> map(F f) const {
        return BidirectionalIteratorView<BidirectionalIteratorView<IteratorType, ViewFunction>, F>(*this, f);
    }
};

}   // namespace spl
