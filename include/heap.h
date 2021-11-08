/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <algorithm>
#include <functional>
#include <cstring>
#include <type_traits>
#include <container.h>
#include <iterator.h>

namespace spl {

using std::move;

/**
 * @brief A sorted heap supporting O(log n) push and pop operations.
 * 
 * @tparam T The type of heap elements.
 * @tparam comp The comparator functor used to sort the heap. The default
 * functor is std::less<T>, which will guarantee that pop() will always return
 * an element that doesn't compare less than any other element; i.e. a max heap.
 */
template <typename T, typename comp = std::less<T>>
class Heap
:   public ForwardIterableContainer<Heap<T, comp>>
{
private:

    static constexpr size_t INITIAL_SIZE = 64;
    static constexpr size_t LINEAR_INCREMENT_THRESHOLD = 64 * 1024 * 1024;

    comp _comp;

    size_t _maxSize = 0;
    size_t _memSize = 0;
    size_t _size = 0;
    T *_data = nullptr;

    template <typename X>
    class HeapIterator
    :   public BidirectionalIterator<HeapIterator<X>, X> {
    private:
        X *_ptr;
    public:

        using reference = typename BidirectionalIterator<HeapIterator<X>, X>::reference;
        using pointer = typename BidirectionalIterator<HeapIterator<X>, X>::pointer;

        HeapIterator(X *ptr)
        : _ptr(ptr)
        { }

        HeapIterator(const HeapIterator &) = default;

        HeapIterator(HeapIterator &&) = default;

        ~HeapIterator() = default;

        HeapIterator & operator=(const HeapIterator &) = default;

        HeapIterator & operator=(HeapIterator &&) = default;

        bool operator==(const HeapIterator &rhs) const { return _ptr == rhs._ptr; }
        bool operator!=(const HeapIterator &rhs) const { return _ptr != rhs._ptr; }
        bool operator< (const HeapIterator &rhs) const { return _ptr <  rhs._ptr; }
        bool operator<=(const HeapIterator &rhs) const { return _ptr <= rhs._ptr; }
        bool operator> (const HeapIterator &rhs) const { return _ptr >  rhs._ptr; }
        bool operator>=(const HeapIterator &rhs) const { return _ptr >= rhs._ptr; }

        reference operator*() {
            return *_ptr;
        }

        pointer operator->() {
            return _ptr;
        }

        HeapIterator & operator++() {
            ++_ptr;
            return *this;
        }

        HeapIterator operator++(int) {
            return _ptr++;
        }

        HeapIterator & operator--() {
            --_ptr;
            return *this;
        }

        HeapIterator operator--(int) {
            return _ptr--;
        }
    };

    void _expand() {
        if (_size == _maxSize) {
            _maxSize = _maxSize >= LINEAR_INCREMENT_THRESHOLD
                ? _maxSize + LINEAR_INCREMENT_THRESHOLD
                : _maxSize * 2;
            _memSize = _maxSize * sizeof(T);
            _data = (T *) realloc(_data, _memSize);
        }
    }

    void _shrink() {
        if (_size <= _maxSize / 2 && _maxSize > INITIAL_SIZE) {
            _maxSize = _maxSize / 2;
            _memSize = _maxSize * sizeof(T);
            _data = (T *) realloc(_data, _memSize);
        }
    }

    void _allocate(size_t size) {
        _maxSize = INITIAL_SIZE;
        while (_maxSize < size) {
            _maxSize = _maxSize >= LINEAR_INCREMENT_THRESHOLD
                ? _maxSize + LINEAR_INCREMENT_THRESHOLD
                : _maxSize * 2;
        }
        _memSize = _maxSize * sizeof(T);
        _size = 0;
        _data = (T *) malloc(_memSize);
    }

    void _invalidate() {
        _maxSize = 0;
        _memSize = 0;
        _size = 0;
        _data = nullptr;
    }

    void _free() {
        if (_data != nullptr) {
            for (size_t i = 0; i < _size; ++i) {
                _data[i].~T();
            }
            free(_data);
        }
    }

    void _copy(const Heap &rhs) {
        _maxSize = rhs._maxSize;
        _memSize = rhs._memSize;
        _size = rhs._size;
        _data = (T *) malloc(_memSize);
        for (size_t i = 0; i < _size; ++i) {
            new (_data + i) T(rhs._data[i]);
        }
    }

    void _move(Heap &rhs) {
        _maxSize = rhs._maxSize;
        _memSize = rhs._memSize;
        _size = rhs._size;
        _data = rhs._data;
    }

    template <typename Begin, typename End>
    void _copy(const Begin &begin, const End &end) {
        for (auto it = begin; it != end; ++it) {
            _expand();
            new (_data + _size) T(*it);
            ++_size;
        }
        std::make_heap(_data, _data + _size, _comp);
    }

    template <typename Begin, typename End>
    void _move(const Begin &begin, const End &end) {
        for (auto it = begin; it != end; ++it) {
            _expand();
            new (_data + _size) T(move(*it));
            ++_size;
        }
        std::make_heap(_data, _data + _size, _comp);
    }

public:

    using Iterator = HeapIterator<T>;
    using ConstIterator = HeapIterator<const T>;

    /**
     * @brief Construct a new Heap object.
     * 
     * @param initialSize The size of the internal storage. This value could be
     * used to efficiently pre-allocate space, if the number of elements (or an
     * estimate) is known.
     */
    Heap(size_t initialSize = INITIAL_SIZE) {
        _allocate(initialSize);
    }

    Heap(const Heap &rhs) {
        _copy(rhs);
    }

    Heap(Heap &&rhs) {
        _move(rhs);
        rhs._invalidate();
    }

    /**
     * @brief Construct a new Heap object.
     * 
     * @param list An initializer list of objects of type T.
     */
    Heap(const std::initializer_list<T> &list) {
        _allocate(list.size());
        _copy(list.begin(), list.end());
    }

    /**
     * @brief Construct a new Heap object.
     * 
     * @param seq An iterable container of type T objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_base_of_v<Heap<T, comp>, Sequence>, int> = 0
    >
    Heap(const Sequence &seq) {
        _allocate(seq.size());
        _copy(seq.begin(), seq.end());
    }

    /**
     * @brief Construct a new Heap object.
     * 
     * @param seq An iterable container of type T objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_base_of_v<Heap<T, comp>, Sequence>, int> = 0
    >
    Heap(Sequence &&seq) {
        _allocate(seq.size());
        _move(seq.begin(), seq.end());
    }

    /**
     * @brief Construct a new Heap object.
     * 
     * @param begin A beginning iterator over type T objects.
     * @param end An end iterator over type T objects.
     */
    template <typename Begin, typename End>
    Heap(const Begin &begin, const End &end) {
        _allocate(INITIAL_SIZE);
        _copy(begin, end);
    }

    /**
     * @brief Construct a new Heap object.
     * 
     * @param begin A beginning iterator over type T objects.
     * @param end An end iterator over type T objects.
     * @param size The number of elements between begin and end.
     */
    template <typename Begin, typename End>
    Heap(const Begin &begin, const End &end, size_t size) {
        _allocate(size);
        _copy(begin, end);
    }

    /**
     * @brief Construct a new Heap object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @return A new Heap object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static Heap<typename It::value_type> create(const It &begin, const EndIt &end) {
        return Heap<typename It::value_type>(begin, end);
    }

    /**
     * @brief Construct a new Deque object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @param size The number of elements between begin and end.
     * @return A new Deque object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static Heap<typename It::value_type> create(const It &begin, const EndIt &end, size_t size) {
        return Heap<typename It::value_type>(begin, end, size);
    }

    ~Heap() {
        _free();
    }

    Heap & operator=(const Heap &rhs) {
        if (this != &rhs) {
            _free();
            _copy(rhs);
        }
        return *this;
    }

    Heap & operator=(Heap &&rhs) {
        if (this != &rhs) {
            _free();
            _move(rhs);
            rhs._invalidate();
        }
        return *this;
    }

    /**
     * @return The size of this container.
     */
    size_t size() const {
        return _size;
    }

    /**
     * @return A boolean indicating whether this container is empty.
     */
    bool empty() const {
        return _size == 0;
    }

    /**
     * @return A boolean indicating whether this container is non-empty.
     */
    bool nonEmpty() const {
        return _size != 0;
    }

    /**
     * @return A constant iterator pointing to the beginning of this container.
     */
    ConstIterator cbegin() const {
        return _data;
    }

    /**
     * @return A constant iterator pointing to a past-the-end position.
     */
    ConstIterator cend() const {
        return _data + _size;
    }

    /**
     * @return An iterator pointing to the beginning of this container.
     */
    Iterator begin() {
        return _data;
    }

    /**
     * @return A constant iterator pointing to the beginning of this container.
     */
    ConstIterator begin() const {
        return cbegin();
    }

    /**
     * @return An iterator pointing to a past-the-end position.
     */
    Iterator end() {
        return _data + _size;
    }

    /**
     * @return A constant iterator pointing to a past-the-end position.
     */
    ConstIterator end() const {
        return cend();
    }

    /**
     * @return A reference to the top element of this heap.
     */
    T & top() {
        return *_data;
    }

    /**
     * @return A constant reference to the top element of this heap.
     */
    const T & top() const {
        return *_data;
    }

    /**
     * @brief Pushes an element onto the heap.
     * 
     * @param[in] elem The element to push.
     * @return A reference to this container for chaining.
     */
    Heap & push(const T &elem) {
        _expand();
        new (_data + _size) T(elem);
        ++_size;
        std::push_heap(_data, _data + _size, _comp);
        return *this;
    }

    /**
     * @brief Pushes an element onto the heap.
     * 
     * @param [in] elem The element to push.
     * @return A reference to this container for chaining.
     */
    Heap & push(T &&elem) {
        _expand();
        new (_data + _size) T(move(elem));
        ++_size;
        std::push_heap(_data, _data + _size, _comp);
        return *this;
    }

    /**
     * @brief Pushes an element onto the heap.
     * 
     * @param[in] elem The element to push.
     * @return A reference to this container for chaining.
     */
    Heap & operator<<(const T &elem) {
        return push(elem);
    }

    /**
     * @brief Pushes an element onto the heap.
     * 
     * @param[in] elem The element to push.
     * @return A reference to this container for chaining.
     */
    Heap & operator<<(T &&elem) {
        return push(move(elem));
    }

    /**
     * @brief Pops the top element from the heap and returns it.
     * 
     * @return The removed top element from the heap.
     */
    T pop() {
        std::pop_heap(_data, _data + _size, _comp);
        --_size;
        T elem = move(_data[_size]);
        _data[_size].~T();
        _shrink();
        return elem;
    }

    /**
     * @brief Pops the top element from the heap and returns it.
     * 
     * @param[out] elem A reference to an element.
     * @return A reference to this container for chaining.
     */
    Heap & pop(T &elem) {
        elem = pop();
        return *this;
    }

    /**
     * @brief Pops the top element from the heap and returns it.
     * 
     * @param[out] elem A reference to an element.
     * @return A reference to this container for chaining.
     */
    Heap & operator>>(T &elem) {
        elem = pop();
        return *this;
    }
};

}
