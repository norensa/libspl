/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <initializer_list>

#include <core/linked_list.h>
#include <container.h>
#include <thread.h>
#include <mutex>
#include <serialization.h>

namespace spl {

/**
 * @brief An exception used for timeout events on a dequeue operation.
*/
struct DequeueTimedout { };

/**
 * @brief Double-ended queue supporting O(1) enqueue and dequeue operations.
 * 
 * @tparam T The type of queue elements.
*/
template <typename T>
class Deque
:   protected __LinkedList::ListBase<T, __LinkedList::SinglyLinkedNode<T>, size_t>,
    public ForwardIterableContainer<Deque<T>>,
    public Serializable
{

    template <typename DequeType> friend struct DequeTester;

protected:
    using base = typename __LinkedList::ListBase<T, __LinkedList::SinglyLinkedNode<T>, size_t>;
    using node = typename base::node;
    using base::_head;
    using base::_tail;
    using base::_mkNode;

public:

    using Iterator = typename base::template ListForwardIterator<T>;
    using ConstIterator = typename base::template ListForwardIterator<const T>;

    /**
     * @brief Construct a new Deque object.
     */
    Deque()
    :   base()
    { }

    Deque(const Deque &rhs)
    :   base(rhs)
    { }

    Deque(Deque &&rhs)
    :   base(std::move(rhs))
    { }

    /**
     * @brief Construct a new Deque object.
     * 
     * @param list An initializer list of objects of type T.
     */
    Deque(const std::initializer_list<T> &list)
    :   base(list)
    { }

    /**
     * @brief Construct a new Deque object.
     * 
     * @param seq An iterable container of type T objects.
     */
    template <typename Sequence>
    Deque(const Sequence &seq)
    :   base(seq)
    { }

    /**
     * @brief Construct a new Deque object.
     * 
     * @param seq An iterable container of type T objects.
     */
    template <typename Sequence>
    Deque(Sequence &&seq)
    :   base(std::move(seq))
    { }

    /**
     * @brief Construct a new Deque object.
     * 
     * @param begin A beginning iterator over type T objects.
     * @param end An end iterator over type T objects.
     */
    template <typename Begin, typename End>
    Deque(const Begin &begin, const End &end)
    :   base(begin, end)
    { }

    /**
     * @brief Construct a new Deque object.
     * 
     * @param begin A beginning iterator over type T objects.
     * @param end An end iterator over type T objects.
     * @param size The number of elements between begin and end.
     */
    template <typename Begin, typename End>
    Deque(const Begin &begin, const End &end, size_t size)
    :   base(begin, end)
    { }

    /**
     * @brief Construct a new Deque object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @return A new Deque object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static Deque<typename It::value_type> create(const It &begin, const EndIt &end) {
        return Deque<typename It::value_type>(begin, end);
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
    static Deque<typename It::value_type> create(const It &begin, const EndIt &end, size_t size) {
        return Deque<typename It::value_type>(begin, end, size);
    }

    ~Deque() = default;

    Deque & operator=(const Deque &rhs) {
        base::operator=(rhs);
        return *this;
    }

    Deque & operator=(Deque &&rhs) {
        base::operator=(std::move(rhs));
        return *this;
    }

    void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
        base::_serialize(serializer);
    }

    void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
        base::_deserialize(serializer);
    }

    /**
     * @return The size of this container.
     */
    size_t size() const {
        return base::size();
    }

    /**
     * @return A boolean indicating whether this container is empty.
     */
    bool empty() const {
        return base::empty();
    }

    /**
     * @return A boolean indicating whether this container is non-empty.
     */
    bool nonEmpty() const {
        return ! base::empty();
    }

    /**
     * @return A constant iterator pointing to the beginning of this container.
     */
    ConstIterator cbegin() const {
        return base::cbegin();
    }

    /**
     * @return A constant iterator pointing to a past-the-end position.
     */
    ConstIterator cend() const {
        return base::cend();
    }

    /**
     * @return An iterator pointing to the beginning of this container.
     */
    Iterator begin() {
        return base::begin();
    }

    /**
     * @return A constant iterator pointing to the beginning of this container.
     */
    ConstIterator begin() const {
        return base::begin();
    }

    /**
     * @return An iterator pointing to a past-the-end position.
     */
    Iterator end() {
        return base::end();
    }

    /**
     * @return A constant iterator pointing to a past-the-end position.
     */
    ConstIterator end() const {
        return base::end();
    }

    /**
     * @return A reference to the first element of this container.
     */
    T & front() {
        return static_cast<node *>(_head)->data;
    }

    /**
     * @return A constant reference to the first element of this container.
     */
    const T & front() const {
        return static_cast<node *>(_head)->data;
    }

    /**
     * @return A reference to the last element of this container.
     */
    T & back() {
        return static_cast<node *>(_tail)->data;
    }

    /**
     * @return A constant reference to the last element of this container.
     */
    const T & back() const {
        return static_cast<node *>(_tail)->data;
    }

    /**
     * @brief Erases all elements in this container.
     * 
     * @return A reference to this container for chaining.
     */
    Deque & clear() {
        base::clear();
        return *this;
    }

    /**
     * @brief Enqueues an element to the front of the queue.
     * 
     * @param[in] elem An element to enqueue.
     * @return A reference to this container for chaining.
     */
    Deque & enqueueFront(const T &elem) {
        base::prepend(_mkNode(elem));
        return *this;
    }

    /**
     * @brief Enqueues an element to the front of the queue.
     * 
     * @param[in] elem An element to enqueue.
     * @return A reference to this container for chaining.
     */
    Deque & enqueueFront(T &&elem) {
        base::prepend(_mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Enqueues an element to the back of the queue.
     * 
     * @param[in] elem An element to enqueue.
     * @return A reference to this container for chaining.
     */
    Deque & enqueue(const T &elem) {
        base::append(_mkNode(elem));
        return *this;
    }

    /**
     * @brief Enqueues an element to the back of the queue.
     * 
     * @param[in] elem An element to enqueue.
     * @return A reference to this container for chaining.
     */
    Deque & enqueue(T &&elem) {
        base::append(_mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Enqueues an element to the back of the queue.
     * 
     * @param[in] elem An element to enqueue.
     * @return A reference to this container for chaining.
     */
    Deque & operator<<(const T &elem) {
        base::append(_mkNode(elem));
        return *this;
    }

    /**
     * @brief Enqueues an element to the back of the queue.
     * 
     * @param[in] elem An element to enqueue.
     * @return A reference to this container for chaining.
     */
    Deque & operator<<(T &&elem) {
        base::append(_mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Dequeues an element from the front of the queue.
     * 
     * @return The element at the front of the queue.
     */
    T dequeue() {
        return base::takeFront();
    }

    /**
     * @brief Dequeues an element from the front of the queue.
     * 
     * @param[out] elem A reference to an element.
     * @return A reference to this container for chaining.
     */
    Deque & dequeue(T &elem) {
        elem = base::takeFront();
        return *this;
    }

    /**
     * @brief Dequeues an element from the front of the queue.
     * 
     * @param[out] elem A reference to an element.
     * @return A reference to this container for chaining.
     */
    Deque & operator>>(T &elem) {
        elem = base::takeFront();
        return *this;
    }

    /**
     * @brief Inserts an element directly before an iterator position.
     * 
     * @param[in] pos An iterator marking point before which the inserted
     * element will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    Deque & insertBefore(const Iterator &pos, const T &elem) {
        base::insertBefore(pos, _mkNode(elem));
        return *this;
    }

    /**
     * @brief Inserts an element directly before an iterator position.
     * 
     * @param[in] pos An iterator marking point before which the inserted
     * element will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    Deque & insertBefore(const Iterator &pos, T &&elem) {
        base::insertBefore(pos, _mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Inserts an element directly after an iterator position.
     * 
     * @param[in] pos An iterator marking point after which the inserted element
     * will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    Deque & insertAfter(const Iterator &pos, const T &elem) {
        base::insertAfter(pos, _mkNode(elem));
        return *this;
    }

    /**
     * @brief Inserts an element directly before an iterator position.
     * 
     * @param[in] pos An iterator marking point before which the inserted
     * element will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    Deque & insertAfter(const Iterator &pos, T &&elem) {
        base::insertAfter(pos, _mkNode(std::move(elem)));
        return *this;
    }

    /**
     * @brief Erases an element at the indicated position.
     * 
     * @param pos An iterator pointing to the element to be erased. The
     * iterator will be moved to the next element before the element is erased.
     * @return A reference to this container for chaining.
     */
    Deque & erase(Iterator &pos) {
        base::erase(pos);
        return *this;
    }

    /**
     * @brief Erases an element at the indicated position.
     * 
     * @param pos An iterator pointing to the element to be erased. The
     * iterator will be moved to the next element before the element is erased.
     * @return A reference to this container for chaining.
     */
    Deque & erase(Iterator &&pos) {
        base::erase(pos);
        return *this;
    }

    /**
     * @brief Removes an element from the indicated position and returns it.
     * 
     * @param pos An iterator pointing to the element to be removed. The
     * iterator will be moved to the next element before the element is removed.
     * @return The removed element.
     */
    T remove(Iterator &pos) {
        return base::remove(pos);
    }

    /**
     * @brief Removes an element from the indicated position and returns it.
     * 
     * @param pos An iterator pointing to the element to be removed. The
     * iterator will be moved to the next element before the element is removed.
     * @return The removed element.
     */
    T remove(Iterator &&pos) {
        return base::remove(pos);
    }
};

namespace parallel
{

/**
 * @brief A double-ended queue with support for O(1), thread-safe enqueue and
 * dequeue operations.
 * 
 * @tparam T The type of queue elements.
*/
template <typename T>
class Deque
:   protected __LinkedList::ListBase<T, __LinkedList::SinglyLinkedNode<T>, size_t>,
    public ForwardIterableContainer<Deque<T>>,
    public Serializable
{

    template <typename DequeType> friend struct DequeTester;

protected:
    using base = typename __LinkedList::ListBase<T, __LinkedList::SinglyLinkedNode<T>, size_t>;
    using node = typename base::node;
    using base::_head;
    using base::_tail;
    using base::_mkNode;
    using base::_size;

private:
    Semaphore _sem;
    std::mutex _mtx;

public:

    using Iterator = typename base::template ListForwardIterator<T>;
    using ConstIterator = typename base::template ListForwardIterator<const T>;

    /**
     * @brief Construct a new Deque object.
     */
    Deque()
    :   base()
    { }

    Deque(const Deque &rhs)
    :   base(rhs),
        _sem((int32_t) _size)
    { }

    Deque(Deque &&rhs)
    :   base(std::move(rhs)),
        _sem((int32_t) _size)
    { }

    /**
     * @brief Construct a new Deque object.
     * 
     * @param list An initializer list of objects of type T.
     */
    Deque(const std::initializer_list<T> &list)
    :   base(list),
        _sem((int32_t) _size)
    { }

    /**
     * @brief Construct a new Deque object.
     * 
     * @param seq An iterable container of type T objects.
     */
    template <typename Sequence>
    Deque(const Sequence &seq)
    :   base(seq),
        _sem((int32_t) _size)
    { }

    /**
     * @brief Construct a new Deque object.
     * 
     * @param seq An iterable container of type T objects.
     */
    template <typename Sequence>
    Deque(Sequence &&seq)
    :   base(std::move(seq)),
        _sem((int32_t) _size)
    { }

    /**
     * @brief Construct a new Deque object.
     * 
     * @param begin A beginning iterator over type T objects.
     * @param end An end iterator over type T objects.
     */
    template <typename Begin, typename End>
    Deque(const Begin &begin, const End &end)
    :   base(begin, end),
        _sem((int32_t) _size)
    { }

    /**
     * @brief Construct a new Deque object.
     * 
     * @param begin A beginning iterator over type T objects.
     * @param end An end iterator over type T objects.
     * @param size The number of elements between begin and end.
     */
    template <typename Begin, typename End>
    Deque(const Begin &begin, const End &end, size_t size)
    :   base(begin, end),
        _sem((int32_t) _size)
    { }

    /**
     * @brief Construct a new Deque object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @return A new Deque object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static Deque<typename It::value_type> create(const It &begin, const EndIt &end) {
        return Deque<typename It::value_type>(begin, end);
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
    static Deque<typename It::value_type> create(const It &begin, const EndIt &end, size_t size) {
        return Deque<typename It::value_type>(begin, end, size);
    }

    ~Deque() = default;

    Deque & operator=(const Deque &rhs) {
        base::operator=(rhs);
        _sem = (int32_t) _size;
        return *this;
    }

    Deque & operator=(Deque &&rhs) {
        base::operator=(std::move(rhs));
        _sem = (int32_t) _size;
        return *this;
    }

    void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
        base::_serialize(serializer);
    }

    void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
        base::_deserialize(serializer);
    }

    /**
     * @return The size of this container.
     */
    size_t size() const {
        return base::size();
    }

    /**
     * @return A boolean indicating whether this container is empty.
     */
    bool empty() const {
        return base::empty();
    }

    /**
     * @return A boolean indicating whether this container is non-empty.
     */
    bool nonEmpty() const {
        return ! base::empty();
    }

    /**
     * Note: This function is not thread-safe.
     * 
     * @return A constant iterator pointing to the beginning of this container.
     */
    ConstIterator cbegin() const {
        return base::cbegin();
    }

    /**
     * Note: This function is not thread-safe.
     * 
     * @return A constant iterator pointing to a past-the-end position.
     */
    ConstIterator cend() const {
        return base::cend();
    }

    /**
     * Note: This function is not thread-safe.
     * 
     * @return An iterator pointing to the beginning of this container.
     */
    Iterator begin() {
        return base::begin();
    }

    /**
     * Note: This function is not thread-safe.
     * 
     * @return A constant iterator pointing to the beginning of this container.
     */
    ConstIterator begin() const {
        return base::begin();
    }

    /**
     * Note: This function is not thread-safe.
     * 
     * @return An iterator pointing to a past-the-end position.
     */
    Iterator end() {
        return base::end();
    }

    /**
     * Note: This function is not thread-safe.
     * 
     * @return A constant iterator pointing to a past-the-end position.
     */
    ConstIterator end() const {
        return base::end();
    }

    /**
     * Note: This function is not thread-safe.
     * 
     * @return A reference to the first element of this container.
     */
    T & front() {
        return static_cast<node *>(_head)->data;
    }

    /**
     * Note: This function is not thread-safe.
     * 
     * @return A constant reference to the first element of this container.
     */
    const T & front() const {
        return static_cast<node *>(_head)->data;
    }

    /**
     * Note: This function is not thread-safe.
     * 
     * @return A reference to the last element of this container.
     */
    T & back() {
        return static_cast<node *>(_tail)->data;
    }

    /**
     * Note: This function is not thread-safe.
     * 
     * @return A constant reference to the last element of this container.
     */
    const T & back() const {
        return static_cast<node *>(_tail)->data;
    }

    /**
     * @brief Erases all elements in this container.
     * 
     * @return A reference to this container for chaining.
     */
    Deque & clear() {
        _mtx.lock();
        base::clear();
        _mtx.unlock();
        return *this;
    }

    /**
     * @brief Enqueues an element to the front of the queue.
     * 
     * @param[in] elem An element to enqueue.
     * @return A reference to this container for chaining.
     */
    Deque & enqueueFront(const T &elem) {
        node *n = _mkNode(elem);
        _mtx.lock();
        base::prepend(n);
        _mtx.unlock();
        _sem.notify();
        return *this;
    }

    /**
     * @brief Enqueues an element to the front of the queue.
     * 
     * @param[in] elem An element to enqueue.
     * @return A reference to this container for chaining.
     */
    Deque & enqueueFront(T &&elem) {
        node *n = _mkNode(std::move(elem));
        _mtx.lock();
        base::prepend(n);
        _mtx.unlock();
        _sem.notify();
        return *this;
    }

    /**
     * @brief Enqueues an element to the back of the queue.
     * 
     * @param[in] elem An element to enqueue.
     * @return A reference to this container for chaining.
     */
    Deque & enqueue(const T &elem) {
        node *n = _mkNode(elem);
        _mtx.lock();
        base::append(n);
        _mtx.unlock();
        _sem.notify();
        return *this;
    }

    /**
     * @brief Enqueues an element to the back of the queue.
     * 
     * @param[in] elem An element to enqueue.
     * @return A reference to this container for chaining.
     */
    Deque & enqueue(T &&elem) {
        node *n = _mkNode(std::move(elem));
        _mtx.lock();
        base::append(n);
        _mtx.unlock();
        _sem.notify();
        return *this;
    }

    /**
     * @brief Enqueues an element to the back of the queue.
     * 
     * @param[in] elem An element to enqueue.
     * @return A reference to this container for chaining.
     */
    Deque & operator<<(const T &elem) {
        return enqueue(elem);
    }

    /**
     * @brief Enqueues an element to the back of the queue.
     * 
     * @param[in] elem An element to enqueue.
     * @return A reference to this container for chaining.
     */
    Deque & operator<<(T &&elem) {
        return enqueue(std::move(elem));
    }

    /**
     * @brief Dequeues an element from the front of the queue or blocks until an
     * element is available.
     * 
     * @return The element at the front of the queue.
     */
    T dequeue() {
        _sem.wait();
        _mtx.lock();
        T data = base::takeFront();
        _mtx.unlock();
        return data;
    }

    /**
     * @brief Dequeues an element from the front of the queue or blocks until an
     * element is available.
     * 
     * @param[out] elem A reference to an element.
     * @return A reference to this container for chaining.
     */
    Deque & dequeue(T &elem) {
        elem = dequeue();
        return *this;
    }

    /**
     * @brief Dequeues an element from the front of the queue or blocks until an
     * element is available.
     * 
     * @param[out] elem A reference to an element.
     * @return A reference to this container for chaining.
     */
    Deque & operator>>(T &elem) {
        elem = dequeue();
        return *this;
    }

    /**
     * @brief Dequeues an element from the front of the queue or blocks for the
     * indicated timeout duration. If timeout is reached, a DequeueTimeout
     * exception is thrown.
     * 
     * @param[in] timeoutNanos The timeout duration in nanoseconds.
     * @throws A DequeueTimeout exception if timeout is reached.
     * @return The element at the front of the queue.
     */
    T dequeueOrTimeout(uint64_t timeoutNanos = 10000lu) {
        if (! _sem.wait(timeoutNanos)) throw DequeueTimedout();
        _mtx.lock();
        T data = base::takeFront();
        _mtx.unlock();
        return data;
    }

    /**
     * @brief Dequeues an element from the front of the queue or blocks for the
     * indicated timeout duration. If timeout is reached, a DequeueTimeout
     * exception is thrown.
     * 
     * @param[out] elem A reference to an element.
     * @param[in] timeoutNanos The timeout duration in nanoseconds.
     * @throws A DequeueTimeout exception if timeout is reached.
     * @return A reference to this container for chaining.
     */
    Deque & dequeueOrTimeout(T &elem, uint64_t timeoutNanos = 10000lu) {
        elem = dequeueOrTimeout(timeoutNanos);
        return *this;
    }

    /**
     * @brief Attempts to dequeue an element from the front of the queue or
     * returns immediately if no elements are currently available.
     * 
     * @param[in] defaultValue A default value to return if there are no
     * elements to dequeue.
     * @return The element at the front of the queue or defaultValue.
     */
    T tryDequeue(const T &defaultValue) {
        if (! _sem.tryWait()) return defaultValue;
        _mtx.lock();
        T data = base::takeFront();
        _mtx.unlock();
        return data;
    }

    /**
     * @brief Attempts to dequeue an element from the front of the queue or
     * returns immediately if no elements are currently available.
     * 
     * @param[out] elem A reference to an element.
     * @param[in] defaultValue A default value to return if there are no
     * elements to dequeue.
     * @return A reference to this container for chaining.
     */
    Deque & tryDequeue(T &elem, const T &defaultValue) {
        if (_size == 0) return defaultValue;
        elem = tryDequeue(defaultValue);
        return *this;
    }

    /**
     * @brief Inserts an element directly before an iterator position.
     * 
     * @param[in] pos An iterator marking point before which the inserted
     * element will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    Deque & insertBefore(const Iterator &pos, const T &elem) {
        base::insertBefore(pos, _mkNode(elem));
        _sem.notify();
        return *this;
    }

    /**
     * @brief Inserts an element directly before an iterator position.
     * 
     * @param[in] pos An iterator marking point before which the inserted
     * element will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    Deque & insertBefore(const Iterator &pos, T &&elem) {
        base::insertBefore(pos, _mkNode(std::move(elem)));
        _sem.notify();
        return *this;
    }

    /**
     * @brief Inserts an element directly after an iterator position.
     * 
     * @param[in] pos An iterator marking point after which the inserted element
     * will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    Deque & insertAfter(const Iterator &pos, const T &elem) {
        base::insertAfter(pos, _mkNode(elem));
        _sem.notify();
        return *this;
    }

    /**
     * @brief Inserts an element directly after an iterator position.
     * 
     * @param[in] pos An iterator marking point after which the inserted element
     * will be located.
     * @param[in] elem The element to be inserted.
     * @return A reference to this container for chaining.
     */
    Deque & insertAfter(const Iterator &pos, T &&elem) {
        base::insertAfter(pos, _mkNode(std::move(elem)));
        _sem.notify();
        return *this;
    }

    /**
     * @brief Erases an element at the indicated position.
     * 
     * @param pos An iterator pointing to the element to be erased. The
     * iterator will be moved to the next element before the element is erased.
     * @return A reference to this container for chaining.
     */
    Deque & erase(Iterator &pos) {
        _sem.wait();
        base::erase(pos);
        return *this;
    }

    /**
     * @brief Erases an element at the indicated position.
     * 
     * @param pos An iterator pointing to the element to be erased. The
     * iterator will be moved to the next element before the element is erased.
     * @return A reference to this container for chaining.
     */
    Deque & erase(Iterator &&pos) {
        _sem.wait();
        base::erase(pos);
        return *this;
    }

    /**
     * @brief Removes an element from the indicated position and returns it.
     * 
     * @param pos An iterator pointing to the element to be removed. The
     * iterator will be moved to the next element before the element is removed.
     * @return The removed element.
     */
    T remove(Iterator &pos) {
        _sem.wait();
        return base::remove(pos);
    }

    /**
     * @brief Remove an element from the indicated position and returns it.
     * 
     * @param pos An iterator pointing to the element to be removed. The
     * iterator will be moved to the next element before the element is removed.
     * @return The removed element.
     */
    T remove(Iterator &&pos) {
        _sem.wait();
        return base::remove(pos);
    }
};

}   // namespace parallel

}   // namespace spl
