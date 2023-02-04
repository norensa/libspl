/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <map>
#include <iterator.h>
#include <serialization.h>
#include <std_serialization.h>

namespace spl {

/**
 * @brief Interval representing a contigious range of value between a start
 * (inclusive) and an end (exclusive)
 * 
 * @tparam T The type of interval elements.
 */
template <typename T>
struct Interval {
    T start;
    T end;
};

/**
 * @brief A non-contigious range of values made up of multiple contigious
 * Interval<T> objects.
 * 
 * @tparam T The type of interval elements.
 */
template <typename T>
class Range
:   public Serializable
{

private:

    class RangeForwardIterator
    :   public ForwardIterator<RangeForwardIterator, const Interval<T>>
    {

        friend class Range;

    private:

        const std::map<T, Interval<T>> &_intervals;
        typename std::map<T, Interval<T>>::const_iterator _it;

        RangeForwardIterator(
            const std::map<T, Interval<T>> &intervals,
            const typename std::map<T, Interval<T>>::const_iterator &it
        )
        :   _intervals(intervals),
            _it(it)
        { }

        RangeForwardIterator(
            const std::map<T, Interval<T>> &intervals,
            typename std::map<T, Interval<T>>::const_iterator &&it
        )
        :   _intervals(intervals),
            _it(std::move(it))
        { }

    public:

        using reference = typename ForwardIterator<RangeForwardIterator, const Interval<T>>::reference;
        using pointer = typename ForwardIterator<RangeForwardIterator, const Interval<T>>::pointer;

        RangeForwardIterator(const RangeForwardIterator &) = default;

        RangeForwardIterator(RangeForwardIterator &&) = default;

        ~RangeForwardIterator() = default;

        RangeForwardIterator & operator=(const RangeForwardIterator &) = default;

        RangeForwardIterator & operator=(RangeForwardIterator &&) = default;

        bool operator==(const RangeForwardIterator &rhs) const {
            return _it == rhs._it;
        }

        bool operator!=(const RangeForwardIterator &rhs) const {
            return ! operator==(rhs);
        }

        reference operator*() const {
            return _it->second;
        }

        pointer operator->() const {
            return &_it->second;
        }

        RangeForwardIterator & operator++() {
            ++_it;
            return *this;
        }

        RangeForwardIterator operator++(int) {
            RangeForwardIterator current = *this;
            operator++();
            return current;
        }
    };

    std::map<T, Interval<T>> _intervals;

public:

    Range() = default;

    Range(const Range &) = default;

    Range(Range &&) = default;

    ~Range() = default;

    Range & operator=(const Range &) = default;

    Range & operator=(Range &&) = default;

    void writeObject(OutputStreamSerializer &serializer) const override {
        serializer << _intervals;
    }

    void readObject(InputStreamSerializer &serializer) override {
        serializer >> _intervals;
    }

    /**
     * @return A const iterator to the first interval of this range.
     */
    auto begin() const {
        return RangeForwardIterator(_intervals, _intervals.begin());
    }

    /**
     * @return A past-the-end iterator of the intervals of this range.
     */
    auto end() const {
        return RangeForwardIterator(_intervals, _intervals.end());
    }

    /**
     * @return A const iterator to the first interval of this range.
     */
    auto cbegin() const {
        return RangeForwardIterator(_intervals, _intervals.begin());
    }

    /**
     * @return A past-the-end iterator of the intervals of this range.
     */
    auto cend() const {
        return RangeForwardIterator(_intervals, _intervals.end());
    }

    /**
     * @brief Inserts an interval into this range.
     * 
     * @param i The interval to insert.
     * @return A reference to this object for chaining.
     */
    Range & insert(Interval<T> i) {
        auto it = _intervals.lower_bound(i.start);
        if (
            (it == _intervals.end() && it != _intervals.begin())
            || (it != _intervals.end() && i.start < it->second.start)
        ) --it;
        if (
            it != _intervals.end()
            && (it->second.start < i.start || it->second.start == i.start)
        ) {
            while (i.start < it->second.end || i.start == it->second.end) {
                i.start = it->second.start;
                if (i.end < it->second.end) i.end = it->second.end;
                it = _intervals.erase(it);
                if (it == _intervals.begin()) break;
                --it;
            }
        }

        it = _intervals.insert({ i.start, i }).first;
        auto next = it;
        ++next;
        while (
            next != _intervals.end()
            && (next->second.start < it->second.end || next->second.start == it->second.end)
        ) {
            if (it->second.end < next->second.end) it->second.end = next->second.end;
            next = _intervals.erase(next);
        }

        return *this;
    }

    /**
     * @brief Tests if a given value is contained in this range.
     * 
     * @param x The value to test.
     * @return True if the value is considered to be within the range, false
     * otherwise.
     */
    bool contains(const T &x) {
        auto it = _intervals.lower_bound(x);
        if (it != _intervals.end() && it->second.start == x) return true;
        if (it != _intervals.begin()) {
            --it;
            if (it->second.start < x && x < it->second.end) return true;
        }
        return false;
    }

    /**
     * @brief Tests if a given interval is contained in this range.
     * 
     * @param x The interval to test.
     * @return True if the interval is considered to be within the range, false
     * otherwise.
     */
    bool contains(const Interval<T> &x) {
        auto it = _intervals.lower_bound(x.start);
        if (
            it != _intervals.end()
            && it->second.start == x.start
            && (x.end < it->second.end || x.end == it->second.end)
        ) return true;
        if (it != _intervals.begin()) {
            --it;
            if (
                it->second.start < x.start
                && x.start < it->second.end
                && (x.end < it->second.end || x.end == it->second.end)
            ) return true;
        }
        return false;
    }

    /**
     * @brief Computes the set union of two ranges.
     * 
     * @param other The range to include in the union operation.
     * @return A new Range object containing the result.
     */
    Range operator|(const Range &other) const {
        Range r = *this;
        for (const auto & i : other) {
            r.insert(i);
        }
        return r;
    }

    /**
     * @brief Computes the set difference of two ranges.
     * 
     * @param other The range to include in the difference operation.
     * @return A new Range object containing the result.
     */
    Range operator-(const Range &other) const {
        Range r = *this;

        auto it = r._intervals.begin();
        auto end = r._intervals.end();

        auto it2 = other._intervals.begin();
        auto end2 = other._intervals.end();

        while (it != end && it2 != end2) {
            // skip until there's an overlap, or end is reached
            if (it->second.end < it2->second.start || it->second.end == it2->second.start) {
                ++it;
                continue;
            }
            if (it2->second.end < it->second.start || it2->second.end == it->second.start) {
                ++it2;
                continue;
            }

            std::pair<T, Interval<T>> i = *it2;

            // starting before and ending before
            //        |____________|
            // - |__________|
            if (i.second.start < it->second.start && i.second.end < it->second.end) {
                i.second.start = i.second.end;
                i.second.end = it->second.end;
                i.first = i.second.start;

                r._intervals.erase(it);
                it = r._intervals.insert(i).first;
            }
            // starting after and ending after
            //   |_____________|
            // -        |____________|
            else if (it->second.start < i.second.start && it->second.end < i.second.end) {
                // alter the end and don't advance (might overlap with more interval(s))
                it->second.end = i.second.start;
            }
            // starting after and ending before
            //   |_________________|
            // -     |_________|
            else if (it->second.start < i.second.start && i.second.end < it->second.end) {
                auto tmp = it->second.end;
                it->second.end = i.second.start;

                i.second.start = i.second.end;
                i.second.end = tmp;
                i.first = i.second.start;

                it = r._intervals.insert(i).first;
            }
            // starting together and ending before
            //   |_________________|
            // - |____________|
            else if (i.second.start == it->second.start && i.second.end < it->second.end) {
                i.second.start = i.second.end;
                i.second.end = it->second.end;
                i.first = i.second.start;

                r._intervals.erase(it);
                it = r._intervals.insert(i).first;
            }
            // starting after and ending together
            //   |______________|
            // -        |_______|
            else if (it->second.start < i.second.start && i.second.end == it->second.end) {
                it->second.end = i.second.start;
            }
            // starting before or together, and ending after or together (completely covering)
            //       |_____|
            // - |_____________|
            else if (
                (i.second.start < it->second.start || i.second.start == it->second.start)
                && (it->second.end < i.second.end || it->second.end == i.second.end)
            ) {
                it = r._intervals.erase(it);
            }
            else {
                break;
            }
        }

        return r;
    }
};

}
