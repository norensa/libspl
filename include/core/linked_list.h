/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <cstddef>
#include <atomic>
#include <exception.h>
#include <iterator.h>
#include <serialization.h>
#include <type_traits>

namespace spl {

namespace __LinkedList {

template <typename T>
struct SinglyLinkedNode {

    typedef SinglyLinkedNode node;
    typedef SinglyLinkedNode * node_ptr;

    node_ptr next;
    T data;

    SinglyLinkedNode()
    :   next(nullptr)
    { }

    SinglyLinkedNode(const T &rhs)
    :   next(nullptr),
        data(rhs)
    { }

    SinglyLinkedNode(T &&rhs)
    :   next(nullptr),
        data(std::move(rhs))
    { }

    SinglyLinkedNode(const node &rhs) = delete;

    SinglyLinkedNode(node &&rhs) = delete;

    ~SinglyLinkedNode() = default;

    node & operator=(const node &) = delete;

    node & operator=(node &&) = delete;

    static void insert(node *newNode, node_ptr &prevNext) {
        newNode->next = prevNext;
        prevNext = newNode;
    }

    static void insert(node *newNode, node_ptr &prevNext, node_ptr &tail) {
        insert(newNode, prevNext);
        if (newNode->next == nullptr) tail = newNode;
    }

    void insertAfter(node *newNode, node_ptr &tail) {
        insert(newNode, next, tail);
    }

    node * remove(node *prevNode, node_ptr &prevNext, node_ptr &tail) {
        prevNext = next;
        if (prevNext == nullptr) tail = prevNode;
        return this;
    }

    static node * removeFront(node_ptr &head, node_ptr &tail) {
        node *h = head;
        node *n = h->next;
        if (n == nullptr) tail = nullptr;
        head = n;
        return h;
    }
};

template <typename T>
struct AtomicSinglyLinkedNode {

    typedef AtomicSinglyLinkedNode node;
    typedef std::atomic<AtomicSinglyLinkedNode *> node_ptr;

    node_ptr next;
    T data;

    AtomicSinglyLinkedNode()
    :   next(nullptr)
    { }

    AtomicSinglyLinkedNode(const T &rhs)
    :   next(nullptr),
        data(rhs)
    { }

    AtomicSinglyLinkedNode(T &&rhs)
    :   next(nullptr),
        data(std::move(rhs))
    { }

    AtomicSinglyLinkedNode(const node &rhs) = delete;

    AtomicSinglyLinkedNode(node &&rhs) = delete;

    ~AtomicSinglyLinkedNode() = default;

    node & operator=(const node &) = delete;

    node & operator=(node &&) = delete;

    static void insert(node *newNode, node_ptr &prevNext) {
        node *pn = prevNext;
        do {
            newNode->next = pn;
        } while (! prevNext.compare_exchange_weak(
            pn,
            newNode,
            std::memory_order_release,
            std::memory_order_relaxed
        ));
    }

    static void insert(node *newNode, node_ptr &prevNext, node_ptr &tail) {
        insert(newNode, prevNext);

        node *t = nullptr;
        tail.compare_exchange_weak(
            t,
            newNode,
            std::memory_order_release,
            std::memory_order_relaxed
        );

        if (t != nullptr) {
            node *n = t->next;
            while (n != nullptr) {
                tail.compare_exchange_weak(
                    t,
                    n,
                    std::memory_order_release, 
                    std::memory_order_relaxed
                );
                n = t->next;
            }
        }
    }

    void insertAfter(node *newNode, node_ptr &tail) {
        insert(newNode, next, tail);
    }

    node * remove(node *prevNode, node_ptr &prevNext, node_ptr &tail) {
        node *expected = this;
        if (expected != nullptr && prevNext.compare_exchange_weak(
            expected,
            nullptr,
            std::memory_order_release, 
            std::memory_order_relaxed
        )) {
            node *n = next.load(std::memory_order_relaxed);
            prevNext.store(n, std::memory_order_release);
            if (n == nullptr) tail.store(prevNode, std::memory_order_release);
            return this;
        }
        return nullptr;
    }

    static node * removeFront(node_ptr &head, node_ptr &tail) {
        node *h = head.load(std::memory_order_relaxed);
        if (h != nullptr && head.compare_exchange_weak(
            h,
            nullptr,
            std::memory_order_release, 
            std::memory_order_relaxed
        )) {
            node *n = h->next.load(std::memory_order_relaxed);
            if (n == nullptr) tail.store(nullptr, std::memory_order_release);
            head.store(n, std::memory_order_release);
            return h;
        }
        return nullptr;
    }
};


template <typename T, typename node_type, typename size_type>
class ListBase {

protected:

    using node = node_type;
    using node_ptr = typename node::node_ptr;

    node_ptr _head;
    node_ptr _tail;
    size_type _size;

    class IteratorBase {

        friend class ListBase;

    protected:
        node *_prev;
        node *_node;

    public:
        IteratorBase(node *current, node *prev = nullptr)
        :   _prev(prev),
            _node(current)
        { }
    };

    template <typename X>
    class ListForwardIterator
    :   public IteratorBase,
        public ForwardIterator<ListForwardIterator<X>, X> {

        friend class ListBase;

    protected:

        void _skipOne() {
            this->_node = this->_node->next;
        }

        ListForwardIterator(node *current, node *prev = nullptr)
        :   IteratorBase(current, prev)
        { }

    public:

        using reference = typename ForwardIterator<ListForwardIterator<X>, X>::reference;
        using pointer = typename ForwardIterator<ListForwardIterator<X>, X>::pointer;

        ListForwardIterator(const ListForwardIterator &) = default;

        ListForwardIterator(ListForwardIterator &&) = default;

        ~ListForwardIterator() = default;

        ListForwardIterator & operator=(const ListForwardIterator &) = default;

        ListForwardIterator & operator=(ListForwardIterator &&) = default;

        bool operator==(const ListForwardIterator &rhs) const {
            return this->_node == rhs._node;
        }

        bool operator!=(const ListForwardIterator &rhs) const {
            return ! operator==(rhs);
        }

        reference operator*() const {
            return this->_node->data;
        }

        pointer operator->() const {
            return &this->_node->data;
        }

        ListForwardIterator & operator++() {
            this->_prev = this->_node;
            this->_node = this->_node->next;
            return *this;
        }

        ListForwardIterator operator++(int) {
            ListForwardIterator current = *this;
            operator++();
            return current;
        }
    };

    static node * _mkNode(const T &data) {
        return new node(data);
    }

    static node * _mkNode(T &&data) {
        return new node(std::move(data));
    }

    template <typename Begin, typename End>
    void _copy(const Begin &begin, const End &end) {
        if (begin == end) {
            _head = nullptr;
            _tail = nullptr;
            _size = 0;
        }
        else {
            auto it = begin;
            node *n = _mkNode(*it);
            _head = n;
            ++it;
            size_t sz = 1;
            for (node *p = n; it != end; ++it, p = n) {
                n = _mkNode(*it);
                node::insert(n, p->next);
                ++sz;
            }
            _tail = n;
            _size = sz;
        }
    }

    template <typename Begin, typename End>
    void _move(const Begin &begin, const End &end) {
        if (begin == end) {
            _head = nullptr;
            _tail = nullptr;
            _size = 0;
        }
        else {
            auto it = begin;
            node *n = _mkNode(std::move(*it));
            _head = n;
            ++it;
            size_t sz = 1;
            for (node *p = n; it != end; ++it, p = n) {
                n = _mkNode(std::move(*it));
                node::insert(n, p->next);
                ++sz;
            }
            _tail = n;
            _size = sz;
        }
    }

    void _move(ListBase &rhs) {
        _head = (node *) rhs._head;
        _tail = (node *) rhs._tail;
        _size = static_cast<size_t>(rhs._size);
    }

    void _invalidate() {
        _head = nullptr;
        _tail = nullptr;
        _size = 0;
    }

    template <
        typename X = T,
        typename std::enable_if<SupportsSerialization<X>::value, int>::type = 0
    >
    void _serialize(OutputStreamSerializer &serializer) const {
        serializer << static_cast<size_t>(_size);
        for (const auto &elem : *this) {
            serializer << elem;
        }
    }

    template <
        typename X = T,
        typename std::enable_if<! SupportsSerialization<X>::value, int>::type = 0
    >
    void _serialize(OutputStreamSerializer &serializer) const {
        throw DynamicMessageError(
            "Type '", typeid(T).name(), "' cannot be serialized."
        );
    }

    template <
        typename X = T,
        typename std::enable_if<SupportsSerialization<X>::value && std::is_constructible<X>::value, int>::type = 0
    >
    void _deserialize(InputStreamSerializer &serializer) {
        size_t sz;
        serializer >> sz;
        for (size_t i = 0; i < sz; ++i) {
            T elem;
            serializer >> elem;
            append(_mkNode(std::move(elem)));
        }
    }

    template <
        typename X = T,
        typename std::enable_if<! SupportsSerialization<X>::value || ! std::is_constructible<X>::value, int>::type = 0
    >
    void _deserialize(InputStreamSerializer &serializer) {
        throw DynamicMessageError(
            "Type '", typeid(T).name(), "' cannot be deserialized."
        );
    }

public:

    ListBase()
    :   _head(nullptr),
        _tail(nullptr),
        _size(0)
    { }

    ListBase(const ListBase &rhs) {
        _copy(rhs.begin(), rhs.end());
    }

    ListBase(ListBase &&rhs) {
        _move(rhs);
        rhs._invalidate();
    }

    template <
        typename Sequence,
        typename std::enable_if<! std::is_base_of<ListBase<T, node, size_type>, Sequence>::value, int>::type = 0
    >
    ListBase(const Sequence &seq) {
        _copy(seq.begin(), seq.end());
    }

    template <
        typename Sequence,
        typename std::enable_if<! std::is_base_of<ListBase<T, node, size_type>, Sequence>::value, int>::type = 0
    >
    ListBase(Sequence &&seq) {
        _move(seq.begin(), seq.end());
    }

    template <typename Begin, typename End>
    ListBase(const Begin &begin, const End &end) {
        _copy(begin, end);
    }

    ~ListBase() {
        clear();
    }

    ListBase & operator=(const ListBase &rhs) {
        if (this != &rhs) {
            clear();
            _copy(rhs.begin(), rhs.end());
        }
        return *this;
    }

    ListBase & operator=(ListBase &&rhs) {
        if (this != &rhs) {
            clear();
            _move(rhs);
            rhs._invalidate();
        }
        return *this;
    }

    size_t size() const {
        return _size;
    }

    bool empty() const {
        return _head == nullptr;
    }

    ListForwardIterator<const T> cbegin() const {
        return ListForwardIterator<const T>(_head);
    }
    ListForwardIterator<T> begin() {
        return ListForwardIterator<T>(_head);
    }
    ListForwardIterator<const T> begin() const {
        return cbegin();
    }

    ListForwardIterator<const T> cend() const {
        return ListForwardIterator<const T>(nullptr, _tail);
    }
    ListForwardIterator<T> end() {
        return ListForwardIterator<T>(nullptr, _tail);
    }
    ListForwardIterator<const T> end() const {
        return cend();
    }

    void clear() {
        while (_size > 0) {
            --_size;
            delete static_cast<node *>(_head)->remove(nullptr, _head, _tail);
        }
    }

    void prepend(node *n) {
        node::insert(n, _head, _tail);
        ++_size;
    }

    void append(node *n) {
        node *t = _tail;
        if (t == nullptr) node::insert(n, _head, _tail);
        else t->insertAfter(n, _tail);
        ++_size;
    }

    void insertBefore(const IteratorBase &pos, node *n) {
        if (pos._node == nullptr) {
            append(n);
        }
        else {
            node::insert(
                n,
                pos._prev == nullptr ? _head : pos._prev->next,
                _tail
            );
            ++_size;
        }
    }

    void insertAfter(const IteratorBase &pos, node *n) {
        if (pos._node == nullptr) {
            delete n;
            throw OutOfRangeError("Attempt to insert an element after a past-the-end iterator");
        }
        pos._node->insertAfter(n, _tail);
        ++_size;
    }

    void erase(ListForwardIterator<T> &pos) {
        // not thread safe

        if (pos._node == nullptr) {
            throw OutOfRangeError("Attempt to remove an element at a past-the-end iterator");
        }
        auto it = pos;
        pos._skipOne();
        --_size;
        delete it._node->remove(
            it._prev,
            it._prev == nullptr ? _head : it._prev->next,
            _tail
        );
    }

    T remove(ListForwardIterator<T> &pos) {
        // not thread safe

        if (pos._node == nullptr) {
            throw OutOfRangeError("Attempt to remove an element at a past-the-end iterator");
        }
        auto it = pos;
        pos._skipOne();
        --_size;
        auto node = it._node->remove(
            it._prev,
            it._prev == nullptr ? _head : it._prev->next,
            _tail
        );
        T data = std::move(node->data);
        delete node;
        return data;
    }

    T takeFront() {
        // thread safe if guaranteed to have elements to remove

        node *n;
        do {
            n = node::removeFront(_head, _tail);
        } while(n == nullptr);

        T data = std::move(n->data);
        delete n;
        --_size;
        return data;
    }
};

}   // namespace __LinkedList

}   // namespace spl
