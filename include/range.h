#pragma once

#include <map>
#include <iterator.h>
#include <serialization.h>
#include <std_serialization.h>

namespace spl {

template <typename T>
struct Interval {
    T start;
    T end;
};

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

    void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
        serializer << _intervals;
    }

    void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
        serializer >> _intervals;
    }

    auto begin() const {
        return RangeForwardIterator(_intervals, _intervals.begin());
    }

    auto end() const {
        return RangeForwardIterator(_intervals, _intervals.end());
    }

    auto cbegin() const {
        return RangeForwardIterator(_intervals, _intervals.begin());
    }

    auto cend() const {
        return RangeForwardIterator(_intervals, _intervals.end());
    }

    Range & insert(Interval<T> i) {
        auto it = _intervals.lower_bound(i.start);
        if (
            (it == _intervals.end() && it != _intervals.begin())
            || it->second.start > i.start
        ) --it;
        if (it != _intervals.end() && it->second.start <= i.start) {
            while (it->second.end >= i.start) {
                i.start = it->second.start;
                if (it->second.end > i.end) i.end = it->second.end;
                it = _intervals.erase(it);
                if (it == _intervals.begin()) break;
                --it;
            }
        }

        it = _intervals.insert({ i.start, i }).first;
        auto next = it;
        ++next;
        while (next != _intervals.end() && next->second.start <= it->second.end) {
            it->second.end = next->second.end;
            next = _intervals.erase(next);
        }

        return *this;
    }

    bool contains(const T &x) {
        auto it = _intervals.lower_bound(x);
        if (it != _intervals.end() && it->second.start == x) return true;
        if (it != _intervals.begin()) {
            --it;
            if (it->second.start < x && it->second.end > x) return true;
        }
        return false;
    }

    bool contains(const Interval<T> &x) {
        auto it = _intervals.lower_bound(x.start);
        if (it != _intervals.end() && it->second.start == x.start && it->second.end >= x.end) return true;
        if (it != _intervals.begin()) {
            --it;
            if (it->second.start < x.start && it->second.end > x.start && it->second.end >= x.end) return true;
        }
        return false;
    }
};

}
