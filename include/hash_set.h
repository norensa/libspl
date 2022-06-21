/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <core/hash_table.h>
#include <functional>       // std::equal_to
#include <hash.h>
#include <container.h>
#include <exception.h>
#include <serialization.h>

namespace spl {

/**
 * @brief A hash set supporting O(1) lookup, insert, and delete.
 * 
 * @tparam Key The key type.
 * @tparam KeyHash A functor to calculate the hash code of the key type. The
 * default functor is Hash<Key>.
 * @tparam KeyEqual A functor to test for key equality. The default functor is
 * std::equal_to<Key>.
 */
template <
    typename Key,
    typename KeyHash = Hash<Key>,
    typename KeyEqual = std::equal_to<Key>
>
class HashSet
:   protected __HashTable::HashTable<
        Key,
        __HashTable::HashSetNode<Key>,
        KeyHash,
        __HashTable::HashSetNodeKeyEqual<KeyEqual>,
        __HashTable::HashTableController,
        size_t
    >,
    public ForwardIterableContainer<HashSet<Key, KeyHash, KeyEqual>>,
    public Serializable
{

    template <typename HashSetType> friend struct HashSetTester;

private:

    using base = typename __HashTable::HashTable<
        Key,
        __HashTable::HashSetNode<Key>,
        KeyHash,
        __HashTable::HashSetNodeKeyEqual<KeyEqual>,
        __HashTable::HashTableController,
        size_t
    >;

    using container_base = ForwardIterableContainer<HashSet<Key, KeyHash, KeyEqual>>;

    using storage_node = typename base::storage_node;

    using base::__NPOS;
    using base::_table;
    using base::_size;
    using base::_hash;
    using base::_findIndex;
    using base::_findRange;
    using base::_findNext;
    using base::_findOrGetFreeIndex;


public:

    using Iterator = typename base::template HashTableIterator<storage_node>;
    using ConstIterator = typename base::template HashTableIterator<const storage_node>;

    using container_base::foreach;

    /**
     * @brief Construct a new HashSet object.
     */
    HashSet()
    :   base()
    { }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @param initialSize The initial size of the internal hash table. This
     * value could be used to efficiently pre-allocate space, if the number of
     * elements (or an estimate) is known.
     */
    HashSet(size_t initialSize)
    :   base(initialSize)
    { }

    HashSet(const HashSet &rhs)
    :   base(rhs)
    { }

    HashSet(HashSet &&rhs)
    :   base(std::move(rhs))
    { }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @param list An initializer list of objects of type Key.
     */
    HashSet(const std::initializer_list<storage_node> &list)
    :   base(list.size())
    {
        putAll(list);
    }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @param seq An iterable container of type Key objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashSet, Sequence>, int> = 0
    >
    HashSet(const Sequence &seq)
    :   base(seq.size())
    {
        putAll(seq);
    }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @param seq An iterable container of type Key objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashSet, Sequence>, int> = 0
    >
    HashSet(Sequence &&seq)
    :   base(seq.size())
    {
        putAll(std::move(seq));
    }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @param begin A beginning iterator over type Key objects.
     * @param end An end iterator over type Key objects.
     */
    template <typename Begin, typename End>
    HashSet(const Begin &begin, const End &end)
    :   base()
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @param begin A beginning iterator over type Keyobjects.
     * @param end An end iterator over type Key objects.
     * @param size The number of elements between begin and end.
     */
    template <typename Begin, typename End>
    HashSet(const Begin &begin, const End &end, size_t size)
    :   base(size)
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @return A new HashSet object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashSet<typename It::value_type> create(const It &begin, const EndIt &end) {
        return HashSet<typename It::value_type>(begin, end);
    }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @param size The number of elements between begin and end.
     * @return A new HashSet object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashSet<typename It::value_type> create(const It &begin, const EndIt &end, size_t size) {
        return HashSet<typename It::value_type>(begin, end, size);
    }

    ~HashSet() = default;

    HashSet & operator=(const HashSet &rhs) {
        base::operator=(rhs);
        return *this;
    }

    HashSet & operator=(HashSet &&rhs) {
        base::operator=(std::move(rhs));
        return *this;
    }

    void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
        base::_serialize(serializer, level);
    }

    void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
        base::_deserialize(serializer, level);
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
     * @brief Erases all elements in this container.
     * 
     * @return A reference to this container for chaining.
     */
    HashSet & clear() {
        base::clear();
        return *this;
    }

    /**
     * @brief Tests whether some key exists in this set.
     * 
     * @param k The key to search for.
     * @return True if the key exists, false otherwise.
     */
    template <typename K>
    bool contains(const K &k) const {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        return i != __NPOS;
    }

    /**
     * @brief Retrieves the actual key corresponding to some given key. If the
     * given key is not found in the set, an ElementNotFoundError will be
     * thrown.
     * 
     * @param k The key to search for.
     * @throws ElementNotFoundError if the key is not found.
     * @return A copy of the actual key corresponding to the given key.
     */
    template <typename K>
    Key get(const K &k) const {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i != __NPOS) return _table[i].storage.n;
        throw ElementNotFoundError();
    }

    /**
     * @brief Retrieves the actual key corresponding to some given key. If the
     * given key is not found in the set, the default value will be returned.
     * 
     * @param k The key to search for.
     * @param defaultValue The default key to return if the key is not found.
     * @return A copy of the actual key corresponding to the given key, or the
     * default value.
     */
    template <typename K>
    Key getOr(const K &k, const Key &defaultValue) const {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i != __NPOS) return _table[i].storage.n;
        return defaultValue;
    }

    /**
     * @brief Inserts a key.
     * 
     * @param k The key to insert.
     * @return A reference to this container for chaining.
     */
    HashSet & put(const Key &k) {
        size_t h = _hash(k);
        size_t i = _findOrGetFreeIndex(h, k);
        if (! _table[i].occupied()) {
            _table[i].set(h, k);
            ++_size;
        }
        return *this;
    }

    /**
     * @brief Inserts a key.
     * 
     * @param k The key to insert.
     * @return A reference to this container for chaining.
     */
    HashSet & put(Key &&k) {
        size_t h = _hash(k);
        size_t i = _findOrGetFreeIndex(h, k);
        if (! _table[i].occupied()) {
            _table[i].set(h, std::move(k));
            ++_size;
        }
        return *this;
    }

    /**
     * @brief Inserts a range of keys.
     * 
     * @param begin A beginning iterator over type Key objects.
     * @param end An end iterator over type Key objects.
     * @return A reference to this container for chaining.
     */
    template <typename Begin, typename End>
    HashSet & putAll(const Begin &begin, const End &end) {
        auto it = begin;
        while (it != end) {
            put(*it);
            ++it;
        }
        return *this;
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type Key objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashSet & putAll(const Sequence &seq) {
        return putAll(seq.begin(), seq.end());
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type Key objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashSet & putAll(Sequence &&seq) {
        auto it = seq.begin();
        auto end = seq.end();
        while (it != end) {
            put(std::move(*it));
            ++it;
        }
        return *this;
    }

    /**
     * @brief Erases a key from this set. If the key does not exist, the
     * function does nothing.
     * 
     * @param k The key to erase.
     * @return True if a key was erased, false otherwise.
     */
    template <typename K>
    bool erase(const K &k) {
        bool retval = false;
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            _table[i].release();
            --_size;
            retval = true;
        }
        return retval;
    }

    /**
     * @brief Erases a key from this set iff it satisfies a given predicate. If
     * no such key exists, the function does nothing.
     * 
     * @param k The key to erase.
     * @param predicate A predicate functor.
     * @return True if a key was erased, false otherwise.
     */
    template <typename K, typename Pred>
    bool erase(const K &k, Pred predicate) {
        bool retval = false;
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i != __NPOS && predicate(_table[i].storage.n)) {
            _table[i].release();
            --_size;
            retval = true;
        }
        return retval;
    }

    /**
     * @brief Removes a key from this set and returns it. If the key does not
     * exist, an ElementNotFoundError will be thrown.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The actual removed key.
     */
    template <typename K>
    Key remove(const K &k) {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i == __NPOS) throw ElementNotFoundError();
        Key retval = std::move(_table[i].storage.n);
        _table[i].release();
        --_size;
        return retval;
    }

    /**
     * @brief Removes a key from this set and returns it iff it satisfies a
     * given predicate. If no such key exists, an ElementNotFoundError will be
     * thrown.
     * 
     * @param k The key to erase.
     * @param predicate A predicate functor.
     * @throws ElementNotFoundError if the key is not found.
     * @return The actual removed key.
     */
    template <typename K, typename Pred>
    Key remove(const K &k, Pred predicate) {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i == __NPOS || ! predicate(_table[i].storage.n)) throw ElementNotFoundError();
        Key retval = std::move(_table[i].storage.n);
        _table[i].release();
        --_size;
        return retval;
    }

    /**
     * @brief Applies a function to all elements matching a given key.
     * 
     * @param k The key to search for.
     * @param f The function to apply.
     * @return A reference to this container for chaining.
     */
    template <typename K, typename F>
    const HashSet & foreach(const K &k, F f) const {
        size_t h = _hash(k);
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            f(_table[i].storage.n);
        }
        return *this;
    }

    /**
     * @brief Applies a function to all elements matching a given key.
     * 
     * @param k The key to search for.
     * @param f The function to apply.
     * @return A reference to this container for chaining.
     */
    template <typename K, typename F>
    HashSet & foreach(const K &k, F f) {
        size_t h = _hash(k);
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            f(_table[i].storage.n);
        }
        return *this;
    }
};

namespace parallel {

/**
 * @brief A hash set supporting thread-safe, O(1) lookup, insert, and delete.
 * 
 * @tparam Key The key type.
 * @tparam KeyHash A functor to calculate the hash code of the key type. The
 * default functor is Hash<Key>.
 * @tparam KeyEqual A functor to test for key equality. The default functor is
 * std::equal_to<Key>.
 */
template <
    typename Key,
    typename KeyHash = Hash<Key>,
    typename KeyEqual = std::equal_to<Key>
>
class HashSet
:   protected __HashTable::HashTable<
        Key,
        __HashTable::AtomicHashSetNode<Key>,
        KeyHash,
        __HashTable::HashSetNodeKeyEqual<KeyEqual>,
        __HashTable::ConcurrentHashTableController,
        std::atomic_size_t
    >,
    public ForwardIterableContainer<HashSet<Key, KeyHash, KeyEqual>>,
    public Serializable
{

    template <typename HashSetType> friend struct HashSetTester;

private:

    using base = typename __HashTable::HashTable<
        Key,
        __HashTable::AtomicHashSetNode<Key>,
        KeyHash,
        __HashTable::HashSetNodeKeyEqual<KeyEqual>,
        __HashTable::ConcurrentHashTableController,
        std::atomic_size_t
    >;

    using container_base = ForwardIterableContainer<HashSet<Key, KeyHash, KeyEqual>>;

    using storage_node = typename base::storage_node;

    using base::__NPOS;
    using base::_controller;
    using base::_table;
    using base::_size;
    using base::_hash;
    using base::_findIndex;
    using base::_findRange;
    using base::_findNext;
    using base::_findOrGetFreeIndex;

public:

    using Iterator = typename base::template HashTableIterator<storage_node>;
    using ConstIterator = typename base::template HashTableIterator<const storage_node>;

    using container_base::foreach;

    /**
     * @brief Construct a new Hash Set object.
     */
    HashSet()
    :   base()
    { }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @param initialSize The initial size of the internal hash table. This
     * value could be used to efficiently pre-allocate space, if the number of
     * elements (or an estimate) is known.
     */
    HashSet(size_t initialSize)
    :   base(initialSize)
    { }

    HashSet(const HashSet &rhs)
    :   base(rhs)
    { }

    HashSet(HashSet &&rhs)
    :   base(std::move(rhs))
    { }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @param list An initializer list of objects of type Key.
     */
    HashSet(const std::initializer_list<storage_node> &list)
    :   base(list.size())
    {
        putAll(list);
    }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @param seq An iterable container of type Key objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashSet, Sequence>, int> = 0
    >
    HashSet(const Sequence &seq)
    :   base(seq.size())
    {
        putAll(seq);
    }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @param seq An iterable container of type Key objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashSet, Sequence>, int> = 0
    >
    HashSet(Sequence &&seq)
    :   base(seq.size())
    {
        putAll(std::move(seq));
    }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @param begin A beginning iterator over type Key objects.
     * @param end An end iterator over type Key objects.
     */
    template <typename Begin, typename End>
    HashSet(const Begin &begin, const End &end)
    :   base()
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @param begin A beginning iterator over type Keyobjects.
     * @param end An end iterator over type Key objects.
     * @param size The number of elements between begin and end.
     */
    template <typename Begin, typename End>
    HashSet(const Begin &begin, const End &end, size_t size)
    :   base(size)
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @return A new HashSet object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashSet<typename It::value_type> create(const It &begin, const EndIt &end) {
        return HashSet<typename It::value_type>(begin, end);
    }

    /**
     * @brief Construct a new HashSet object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @param size The number of elements between begin and end.
     * @return A new HashSet object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashSet<typename It::value_type> create(const It &begin, const EndIt &end, size_t size) {
        return HashSet<typename It::value_type>(begin, end, size);
    }

    ~HashSet() = default;

    HashSet & operator=(const HashSet &rhs) {
        base::operator=(rhs);
        return *this;
    }

    HashSet & operator=(HashSet &&rhs) {
        base::operator=(std::move(rhs));
        return *this;
    }

    void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
        base::_serialize(serializer, level);
    }

    void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
        base::_deserialize(serializer, level);
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

    HashSet & clear() {
        base::clear();
        return *this;
    }

    /**
     * @brief Tests whether some key exists in this set.
     * 
     * @param k The key to search for.
     * @return True if the key exists, false otherwise.
     */
    template <typename K>
    bool contains(const K &k) const {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        _controller.exit();
        return i != __NPOS;
    }

    /**
     * @brief Retrieves the actual key corresponding to some given key. If the
     * given key is not found in the set, an ElementNotFoundError will be
     * thrown.
     * 
     * @param k The key to search for.
     * @throws ElementNotFoundError if the key is not found.
     * @return A copy of the actual key corresponding to the given key.
     */
    template <typename K>
    Key get(const K &k) const {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            Key retval = _table[i].storage.n;
            _controller.exit();
            return retval;
        }
        _controller.exit();
        throw ElementNotFoundError();
    }

    /**
     * @brief Retrieves the actual key corresponding to some given key. If the
     * given key is not found in the set, the default value will be returned.
     * 
     * @param k The key to search for.
     * @param defaultValue The default key to return if the key is not found.
     * @return A copy of the actual key corresponding to the given key, or the
     * default value.
     */
    template <typename K>
    Key getOr(const K &k, const Key &defaultValue) const {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            Key retval = _table[i].storage.n;
            _controller.exit();
            return retval;
        }
        _controller.exit();
        return defaultValue;
    }

    /**
     * @brief Inserts a key.
     * 
     * @param k The key to insert.
     * @return A reference to this container for chaining.
     */
    HashSet & put(const Key &k) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findOrGetFreeIndex(h, k);
        if (! _table[i].occupied()) {
            _table[i].set(h, k);
            ++_size;
        }
        _controller.exit();
        return *this;
    }

    /**
     * @brief Inserts a key.
     * 
     * @param k The key to insert.
     * @return A reference to this container for chaining.
     */
    HashSet & put(Key &&k) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findOrGetFreeIndex(h, k);
        if (! _table[i].occupied()) {
            _table[i].set(h, std::move(k));
            ++_size;
        }
        _controller.exit();
        return *this;
    }

    /**
     * @brief Inserts a range of keys.
     * 
     * @param begin A beginning iterator over type Key objects.
     * @param end An end iterator over type Key objects.
     * @return A reference to this container for chaining.
     */
    template <typename Begin, typename End>
    HashSet & putAll(const Begin &begin, const End &end) {
        auto it = begin;
        while (it != end) {
            put(*it);
            ++it;
        }
        return *this;
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type Key objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashSet & putAll(const Sequence &seq) {
        return putAll(seq.begin(), seq.end());
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type Key objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashSet & putAll(Sequence &&seq) {
        auto it = seq.begin();
        auto end = seq.end();
        while (it != end) {
            put(std::move(*it));
            ++it;
        }
        return *this;
    }

    /**
     * @brief Erases a key from this set. If the key does not exist, the
     * function does nothing.
     * 
     * @param k The key to erase.
     * @return True if a key was erased, false otherwise.
     */
    template <typename K>
    bool erase(const K &k) {
        bool retval = false;
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            _table[i].release();
            --_size;
            retval = true;
        }
        _controller.exit();
        return retval;
    }

    /**
     * @brief Erases a key from this set. If the key does not exist, the
     * function does nothing.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.
     * 
     * @param k The key to erase.
     * @return True if a key was erased, false otherwise.
     */
    template <typename K>
    bool erase_l(const K &k) {
        bool retval = false;
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            _table[i].release();
            --_size;
            retval = true;
        }
        _controller.unlock();
        _controller.exit();
        return retval;
    }

    /**
     * @brief Erases a key from this set iff it satisfies a given predicate. If
     * no such key exists, the function does nothing.
     * 
     * @param k The key to erase.
     * @param predicate A predicate functor.
     * @return True if a key was erased, false otherwise.
     */
    template <typename K, typename Pred>
    bool erase(const K &k, Pred predicate) {
        bool retval = false;
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i != __NPOS && predicate(_table[i].storage.n)) {
            _table[i].release();
            --_size;
            retval = true;
        }
        _controller.exit();
        return retval;
    }

    /**
     * @brief Erases a key from this set iff it satisfies a given predicate. If
     * no such key exists, the function does nothing.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.

     * @param k The key to erase.
     * @param predicate A predicate functor.
     * @return True if a key was erased, false otherwise.
     */
    template <typename K, typename Pred>
    bool erase_l(const K &k, Pred predicate) {
        bool retval = false;
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        size_t i = _findIndex(h, k);
        if (i != __NPOS && predicate(_table[i].storage.n)) {
            _table[i].release();
            --_size;
            retval = true;
        }
        _controller.unlock();
        _controller.exit();
        return retval;
    }

    /**
     * @brief Removes a key from this set and returns it. If the key does not
     * exist, an ElementNotFoundError will be thrown.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The actual removed key.
     */
    template <typename K>
    Key remove(const K &k) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i == __NPOS) {
            _controller.exit();
            throw ElementNotFoundError();
        }
        Key retval = std::move(_table[i].storage.n);
        _table[i].release();
        --_size;
        _controller.exit();
        return retval;
    }

    /**
     * @brief Removes a key from this set and returns it. If the key does not
     * exist, an ElementNotFoundError will be thrown.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The actual removed key.
     */
    template <typename K>
    Key remove_l(const K &k) {
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        size_t i = _findIndex(h, k);
        if (i == __NPOS) {
            _controller.unlock();
            _controller.exit();
            throw ElementNotFoundError();
        }
        Key retval = std::move(_table[i].storage.n);
        _table[i].release();
        --_size;
        _controller.unlock();
        _controller.exit();
        return retval;
    }

    /**
     * @brief Removes a key from this set and returns it iff it satisfies a
     * given predicate. If no such key exists, an ElementNotFoundError will be
     * thrown.
     * 
     * @param k The key to erase.
     * @param predicate A predicate functor.
     * @throws ElementNotFoundError if the key is not found.
     * @return The actual removed key.
     */
    template <typename K, typename Pred>
    Key remove(const K &k, Pred predicate) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i == __NPOS || ! predicate(_table[i].storage.n)) {
            _controller.exit();
            throw ElementNotFoundError();
        }
        Key retval = std::move(_table[i].storage.n);
        _table[i].release();
        --_size;
        _controller.exit();
        return retval;
    }

    /**
     * @brief Removes a key from this set and returns it iff it satisfies a
     * given predicate. If no such key exists, an ElementNotFoundError will be
     * thrown.
     * 
     * @param k The key to erase.
     * @param predicate A predicate functor.
     * @throws ElementNotFoundError if the key is not found.
     * @return The actual removed key.
     */
    template <typename K, typename Pred>
    Key remove_l(const K &k, Pred predicate) {
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        size_t i = _findIndex(h, k);
        if (i == __NPOS || ! predicate(_table[i].storage.n)) {
            _controller.unlock();
            _controller.exit();
            throw ElementNotFoundError();
        }
        Key retval = std::move(_table[i].storage.n);
        _table[i].release();
        --_size;
        _controller.unlock();
        _controller.exit();
        return retval;
    }

    /**
     * @brief Applies a function to all elements matching a given key.
     * 
     * @param k The key to search for.
     * @param f The function to apply.
     * @return A reference to this container for chaining.
     */
    template <typename K, typename F>
    const HashSet & foreach(const K &k, F f) const {
        size_t h = _hash(k);
        _controller.enter();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            f(_table[i].storage.n);
        }
        _controller.exit();
        return *this;
    }

    /**
     * @brief Applies a function to all elements matching a given key.
     * 
     * @param k The key to search for.
     * @param f The function to apply.
     * @return A reference to this container for chaining.
     */
    template <typename K, typename F>
    HashSet & foreach(const K &k, F f) {
        size_t h = _hash(k);
        _controller.enter();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            f(_table[i].storage.n);
        }
        _controller.exit();
        return *this;
    }

    /**
     * @brief Applies a function to all elements matching a given key.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.
     * 
     * @param k The key to search for.
     * @param f The function to apply.
     * @return A reference to this container for chaining.
     */
    template <typename K, typename F>
    const HashSet & foreach_l(const K &k, F f) const {
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            f(_table[i].storage.n);
        }
        _controller.unlock();
        _controller.exit();
        return *this;
    }

    /**
     * @brief Applies a function to all elements matching a given key.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.
     * 
     * @param k The key to search for.
     * @param f The function to apply.
     * @return A reference to this container for chaining.
     */
    template <typename K, typename F>
    HashSet & foreach_l(const K &k, F f) {
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            f(_table[i].storage.n);
        }
        _controller.unlock();
        _controller.exit();
        return *this;
    }
};

}   // namespace parallel

/**
 * @brief A hash multi-set supporting O(1) lookup, insert, and delete.
 * 
 * @tparam Key The key type.
 * @tparam KeyHash A functor to calculate the hash code of the key type. The
 * default functor is Hash<Key>.
 * @tparam KeyEqual A functor to test for key equality. The default functor is
 * std::equal_to<Key>.
 */

template <
    typename Key,
    typename KeyHash = Hash<Key>,
    typename KeyEqual = std::equal_to<Key>
>
class HashMultiSet
:   protected __HashTable::HashTable<
        Key,
        __HashTable::HashSetNode<Key>,
        KeyHash,
        __HashTable::HashSetNodeKeyEqual<KeyEqual>,
        __HashTable::HashTableController,
        size_t
    >,
    public ForwardIterableContainer<HashMultiSet<Key, KeyHash, KeyEqual>>,
    public Serializable
{

    template <typename HashSetType> friend struct HashSetTester;

private:

    using base = typename __HashTable::HashTable<
        Key,
        __HashTable::HashSetNode<Key>,
        KeyHash,
        __HashTable::HashSetNodeKeyEqual<KeyEqual>,
        __HashTable::HashTableController,
        size_t
    >;

    using container_base = ForwardIterableContainer<HashMultiSet<Key, KeyHash, KeyEqual>>;

    using storage_node = typename base::storage_node;

    using base::__NPOS;
    using base::_table;
    using base::_size;
    using base::_hash;
    using base::_findIndex;
    using base::_findRange;
    using base::_findNext;
    using base::_getFreeIndex;

public:

    using Iterator = typename base::template HashTableIterator<storage_node>;
    using ConstIterator = typename base::template HashTableIterator<const storage_node>;

    using container_base::foreach;

    /**
     * @brief Construct a new HashMultiSet object.
     */
    HashMultiSet()
    :   base()
    { }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @param initialSize The initial size of the internal hash table. This
     * value could be used to efficiently pre-allocate space, if the number of
     * elements (or an estimate) is known.
     */
    HashMultiSet(size_t initialSize)
    :   base(initialSize)
    { }

    HashMultiSet(const HashMultiSet &rhs)
    :   base(rhs)
    { }

    HashMultiSet(HashMultiSet &&rhs)
    :   base(std::move(rhs))
    { }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @param list An initializer list of objects of type Key.
     */
    HashMultiSet(const std::initializer_list<storage_node> &list)
    :   base(list.size())
    {
        putAll(list);
    }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @param seq An iterable container of type Key objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashMultiSet, Sequence>, int> = 0
    >
    HashMultiSet(const Sequence &seq)
    :   base(seq.size())
    {
        putAll(seq);
    }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @param seq An iterable container of type Key objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashMultiSet, Sequence>, int> = 0
    >
    HashMultiSet(Sequence &&seq)
    :   base(seq.size())
    {
        putAll(std::move(seq));
    }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @param begin A beginning iterator over type Key objects.
     * @param end An end iterator over type Key objects.
     */
    template <typename Begin, typename End>
    HashMultiSet(const Begin &begin, const End &end)
    :   base()
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @param begin A beginning iterator over type Keyobjects.
     * @param end An end iterator over type Key objects.
     * @param size The number of elements between begin and end.
     */
    template <typename Begin, typename End>
    HashMultiSet(const Begin &begin, const End &end, size_t size)
    :   base(size)
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @return A new HashMultiSet object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashMultiSet<typename It::value_type> create(const It &begin, const EndIt &end) {
        return HashMultiSet<typename It::value_type>(begin, end);
    }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @param size The number of elements between begin and end.
     * @return A new HashMultiSet object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashMultiSet<typename It::value_type> create(const It &begin, const EndIt &end, size_t size) {
        return HashMultiSet<typename It::value_type>(begin, end, size);
    }

    ~HashMultiSet() = default;

    HashMultiSet & operator=(const HashMultiSet &rhs) {
        base::operator=(rhs);
        return *this;
    }

    HashMultiSet & operator=(HashMultiSet &&rhs) {
        base::operator=(std::move(rhs));
        return *this;
    }

    void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
        base::_serialize(serializer, level);
    }

    void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
        base::_deserialize(serializer, level);
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
     * @brief Erases all elements in this container.
     * 
     * @return A reference to this container for chaining.
     */
    HashMultiSet & clear() {
        base::clear();
        return *this;
    }

    /**
     * @brief Tests whether some key exists in this set.
     * 
     * @param k The key to search for.
     * @return True if the key exists, false otherwise.
     */
    template <typename K>
    bool contains(const K &k) const {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        return i != __NPOS;
    }

    /**
     * @brief Retrieves an actual key corresponding to some given key. If the
     * given key is not found in the set, an ElementNotFoundError will be
     * thrown.
     * 
     * @param k The key to search for.
     * @throws ElementNotFoundError if the key is not found.
     * @return A copy of an actual key corresponding to the given key.
     */
    template <typename K>
    Key get(const K &k) const {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i != __NPOS) return _table[i].storage.n;
        throw ElementNotFoundError();
    }

    /**
     * @brief Retrieves an actual key corresponding to some given key. If the
     * given key is not found in the set, the default value will be returned.
     * 
     * @param k The key to search for.
     * @param defaultValue The default key to return if the key is not found.
     * @return A copy of an actual key corresponding to the given key, or the
     * default value.
     */
    template <typename K>
    Key getOr(const K &k, const Key &defaultValue) const {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i != __NPOS) return _table[i].storage.n;
        return defaultValue;
    }

    /**
     * @brief Inserts a key.
     * 
     * @param k The key to insert.
     * @return A reference to this container for chaining.
     */
    HashMultiSet & put(const Key &k) {
        size_t h = _hash(k);
        size_t i = _getFreeIndex(h);
        _table[i].set(h, k);
        ++_size;
        return *this;
    }

    /**
     * @brief Inserts a key.
     * 
     * @param k The key to insert.
     * @return A reference to this container for chaining.
     */
    HashMultiSet & put(Key &&k) {
        size_t h = _hash(k);
        size_t i = _getFreeIndex(h);
        _table[i].set(h, std::move(k));
        ++_size;
        return *this;
    }

    /**
     * @brief Inserts a range of keys.
     * 
     * @param begin A beginning iterator over type Key objects.
     * @param end An end iterator over type Key objects.
     * @return A reference to this container for chaining.
     */
    template <typename Begin, typename End>
    HashMultiSet & putAll(const Begin &begin, const End &end) {
        auto it = begin;
        while (it != end) {
            put(*it);
            ++it;
        }
        return *this;
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type Key objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashMultiSet & putAll(const Sequence &seq) {
        return putAll(seq.begin(), seq.end());
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type Key objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashMultiSet & putAll(Sequence &&seq) {
        auto it = seq.begin();
        auto end = seq.end();
        while (it != end) {
            put(std::move(*it));
            ++it;
        }
        return *this;
    }

    /**
     * @brief Erases a key from this set. If the key does not exist, the
     * function does nothing. If more than one key matches the given key, only
     * one key is erased.
     * 
     * @param k The key to erase.
     * @return True if a key was erased, false otherwise.
     */
    template <typename K>
    bool erase(const K &k) {
        bool retval = false;
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            _table[i].release();
            --_size;
            retval = true;
        }
        return retval;
    }

    /**
     * @brief Erases a key from this set iff it satisfies a given predicate. If
     * no such key exists, the function does nothing. If more than one key
     * matches the given key and satisfies the predicate, only one key will be
     * erased.
     * 
     * @param k The key to erase.
     * @param predicate A predicate functor.
     * @return True if a key was erased, false otherwise.
     */
    template <typename K, typename Pred>
    bool erase(const K &k, Pred predicate) {
        bool retval = false;
        size_t h = _hash(k);
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            if (predicate(_table[i].storage.n)) {
                _table[i].release();
                --_size;
                retval = true;
                break;
            }
        }
        return retval;
    }

    /**
     * @brief Erases all keys from this set that match a given key. If no such
     * key exists, the function does nothing.
     * 
     * @param k The key to erase.
     * @return Number of keys erased.
     */
    template <typename K>
    size_t eraseAll(const K &k) {
        size_t retval = 0;
        size_t h = _hash(k);
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            _table[i].release();
            --_size;
            ++retval;
        }
        return retval;
    }

    /**
     * @brief Erases all keys from this set that match a given key and satisfy a
     * given predicate. If no such key exists, the function does nothing.
     * 
     * @param k The key to erase.
     * @param predicate A predicate functor.
     * @return Number of keys erased.
     */
    template <typename K, typename Pred>
    size_t eraseAll(const K &k, Pred predicate) {
        size_t retval = 0;
        size_t h = _hash(k);
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            if (predicate(_table[i].storage.n)) {
                _table[i].release();
                --_size;
                ++retval;
            }
        }
        return retval;
    }

    /**
     * @brief Removes a key from this set and returns it. If the key does not
     * exist, an ElementNotFoundError will be thrown.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The actual removed key.
     */
    template <typename K>
    Key remove(const K &k) {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i == __NPOS) throw ElementNotFoundError();
        Key retval = std::move(_table[i].storage.n);
        _table[i].release();
        --_size;
        return retval;
    }

    /**
     * @brief Removes a key from this set and returns it iff it satisfies a
     * given predicate. If no such key exists, an ElementNotFoundError will be
     * thrown. If more than one key matches the given key and satisfies the
     * predicate, only one key will be erased.
     * 
     * @param k The key to erase.
     * @param predicate A predicate functor.
     * @throws ElementNotFoundError if the key is not found.
     * @return The value corresponding to the erased key.
     */
    template <typename K, typename Pred>
    Key remove(const K &k, Pred predicate) {
        size_t h = _hash(k);
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            if (predicate(_table[i].storage.n)) {
                Key retval = std::move(_table[i].storage.n);
                _table[i].release();
                --_size;
                return retval;
            }
        }
        throw ElementNotFoundError();
    }

    /**
     * @brief Applies a function to all elements matching a given key.
     * 
     * @param k The key to search for.
     * @param f The function to apply.
     * @return A reference to this container for chaining.
     */
    template <typename K, typename F>
    const HashMultiSet & foreach(const K &k, F f) const {
        size_t h = _hash(k);
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            f(_table[i].storage.n);
        }
        return *this;
    }

    /**
     * @brief Applies a function to all elements matching a given key.
     * 
     * @param k The key to search for.
     * @param f The function to apply.
     * @return A reference to this container for chaining.
     */
    template <typename K, typename F>
    HashMultiSet & foreach(const K &k, F f) {
        size_t h = _hash(k);
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            f(_table[i].storage.n);
        }
        return *this;
    }
};

namespace parallel {

/**
 * @brief A hash multi-set supporting thread-safe, O(1) lookup, insert, and
 * delete.
 * 
 * @tparam Key The key type.
 * @tparam KeyHash A functor to calculate the hash code of the key type. The
 * default functor is Hash<Key>.
 * @tparam KeyEqual A functor to test for key equality. The default functor is
 * std::equal_to<Key>.
*/
template <
    typename Key,
    typename KeyHash = Hash<Key>,
    typename KeyEqual = std::equal_to<Key>
>
class HashMultiSet
:   protected __HashTable::HashTable<
        Key,
        __HashTable::AtomicHashSetNode<Key>,
        KeyHash,
        __HashTable::HashSetNodeKeyEqual<KeyEqual>,
        __HashTable::ConcurrentHashTableController,
        std::atomic_size_t
    >,
    public ForwardIterableContainer<HashMultiSet<Key, KeyHash, KeyEqual>>,
    public Serializable
{

    template <typename HashSetType> friend struct HashSetTester;

private:

    using base = typename __HashTable::HashTable<
        Key,
        __HashTable::AtomicHashSetNode<Key>,
        KeyHash,
        __HashTable::HashSetNodeKeyEqual<KeyEqual>,
        __HashTable::ConcurrentHashTableController,
        std::atomic_size_t
    >;

    using container_base = ForwardIterableContainer<HashMultiSet<Key, KeyHash, KeyEqual>>;

    using storage_node = typename base::storage_node;

    using base::__NPOS;
    using base::_controller;
    using base::_table;
    using base::_size;
    using base::_hash;
    using base::_findIndex;
    using base::_findRange;
    using base::_findNext;
    using base::_getFreeIndex;

public:

    using Iterator = typename base::template HashTableIterator<storage_node>;
    using ConstIterator = typename base::template HashTableIterator<const storage_node>;

    using container_base::foreach;

    /**
     * @brief Construct a new HashMultiSet object.
     */
    HashMultiSet()
    :   base()
    { }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @param initialSize The initial size of the internal hash table. This
     * value could be used to efficiently pre-allocate space, if the number of
     * elements (or an estimate) is known.
     */
    HashMultiSet(size_t initialSize)
    :   base(initialSize)
    { }

    HashMultiSet(const HashMultiSet &rhs)
    :   base(rhs)
    { }

    HashMultiSet(HashMultiSet &&rhs)
    :   base(std::move(rhs))
    { }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @param list An initializer list of objects of type Key.
     */
    HashMultiSet(const std::initializer_list<storage_node> &list)
    :   base(list.size())
    {
        putAll(list);
    }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @param seq An iterable container of type Key objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashMultiSet, Sequence>, int> = 0
    >
    HashMultiSet(const Sequence &seq)
    :   base(seq.size())
    {
        putAll(seq);
    }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @param seq An iterable container of type Key objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashMultiSet, Sequence>, int> = 0
    >
    HashMultiSet(Sequence &&seq)
    :   base(seq.size())
    {
        putAll(std::move(seq));
    }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @param begin A beginning iterator over type Key objects.
     * @param end An end iterator over type Key objects.
     */
    template <typename Begin, typename End>
    HashMultiSet(const Begin &begin, const End &end)
    :   base()
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @param begin A beginning iterator over type Keyobjects.
     * @param end An end iterator over type Key objects.
     * @param size The number of elements between begin and end.
     */
    template <typename Begin, typename End>
    HashMultiSet(const Begin &begin, const End &end, size_t size)
    :   base(size)
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @return A new HashMultiSet object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashMultiSet<typename It::value_type> create(const It &begin, const EndIt &end) {
        return HashMultiSet<typename It::value_type>(begin, end);
    }

    /**
     * @brief Construct a new HashMultiSet object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @param size The number of elements between begin and end.
     * @return A new HashMultiSet object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashMultiSet<typename It::value_type> create(const It &begin, const EndIt &end, size_t size) {
        return HashMultiSet<typename It::value_type>(begin, end, size);
    }

    ~HashMultiSet() = default;

    HashMultiSet & operator=(const HashMultiSet &rhs) {
        base::operator=(rhs);
        return *this;
    }

    HashMultiSet & operator=(HashMultiSet &&rhs) {
        base::operator=(std::move(rhs));
        return *this;
    }

    void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
        base::_serialize(serializer, level);
    }

    void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
        base::_deserialize(serializer, level);
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
     * @brief Erases all elements in this container.
     * 
     * @return A reference to this container for chaining.
     */
    HashMultiSet & clear() {
        base::clear();
        return *this;
    }

    /**
     * @brief Tests whether some key exists in this set.
     * 
     * @param k The key to search for.
     * @return True if the key exists, false otherwise.
     */
    template <typename K>
    bool contains(const K &k) const {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        _controller.exit();
        return i != __NPOS;
    }

    /**
     * @brief Retrieves an actual key corresponding to some given key. If the
     * given key is not found in the set, an ElementNotFoundError will be
     * thrown.
     * 
     * @param k The key to search for.
     * @throws ElementNotFoundError if the key is not found.
     * @return A copy of an actual key corresponding to the given key.
     */
    template <typename K>
    Key get(const K &k) const {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            Key retval = _table[i].storage.n;
            _controller.exit();
            return retval;
        }
        _controller.exit();
        throw ElementNotFoundError();
    }

    /**
     * @brief Retrieves an actual key corresponding to some given key. If the
     * given key is not found in the set, the default value will be returned.
     * 
     * @param k The key to search for.
     * @param defaultValue The default key to return if the key is not found.
     * @return A copy of an actual key corresponding to the given key, or the
     * default value.
     */
    template <typename K>
    Key getOr(const K &k, const Key &defaultValue) const {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            Key retval = _table[i].storage.n;
            _controller.exit();
            return retval;
        }
        _controller.exit();
        return defaultValue;
    }

    /**
     * @brief Inserts a key.
     * 
     * @param k The key to insert.
     * @return A reference to this container for chaining.
     */
    HashMultiSet & put(const Key &k) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _getFreeIndex(h);
        _table[i].set(h, k);
        ++_size;
        _controller.exit();
        return *this;
    }

    /**
     * @brief Inserts a key.
     * 
     * @param k The key to insert.
     * @return A reference to this container for chaining.
     */
    HashMultiSet & put(Key &&k) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _getFreeIndex(h);
        _table[i].set(h, std::move(k));
        ++_size;
        _controller.exit();
        return *this;
    }

    /**
     * @brief Inserts a range of keys.
     * 
     * @param begin A beginning iterator over type Key objects.
     * @param end An end iterator over type Key objects.
     * @return A reference to this container for chaining.
     */
    template <typename Begin, typename End>
    HashMultiSet & putAll(const Begin &begin, const End &end) {
        auto it = begin;
        while (it != end) {
            put(*it);
            ++it;
        }
        return *this;
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type Key objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashMultiSet & putAll(const Sequence &seq) {
        return putAll(seq.begin(), seq.end());
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type Key objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashMultiSet & putAll(Sequence &&seq) {
        auto it = seq.begin();
        auto end = seq.end();
        while (it != end) {
            put(std::move(*it));
            ++it;
        }
        return *this;
    }

    /**
     * @brief Erases a key from this set. If the key does not exist, the
     * function does nothing. If more than one key matches the given key, only
     * one key is erased.
     * 
     * @param k The key to erase.
     * @return True if a key was erased, false otherwise.
     */
    template <typename K>
    bool erase(const K &k) {
        bool retval = false;
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            _table[i].release();
            --_size;
            retval = true;
        }
        _controller.exit();
        return retval;
    }

    /**
     * @brief Erases a key from this set. If the key does not exist, the
     * function does nothing. If more than one key matches the given key, only
     * one key is erased.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.

     * @param k The key to erase.
     * @return True if a key was erased, false otherwise.
     */
    template <typename K>
    bool erase_l(const K &k) {
        bool retval = false;
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            _table[i].release();
            --_size;
            retval = true;
        }
        _controller.unlock();
        _controller.exit();
        return retval;
    }

    /**
     * @brief Erases a key from this set iff it satisfies a given predicate. If
     * no such key exists, the function does nothing. If more than one key
     * matches the given key and satisfies the predicate, only one key will be
     * erased.
     * 
     * @param k The key to erase.
     * @param predicate A predicate functor.
     * @return True if a key was erased, false otherwise.
     */
    template <typename K, typename Pred>
    bool erase(const K &k, Pred predicate) {
        bool retval = false;
        size_t h = _hash(k);
        _controller.enter();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            if (predicate(_table[i].storage.n)) {
                _table[i].release();
                --_size;
                retval = true;
                break;
            }
        }
        _controller.exit();
        return retval;
    }

    /**
     * @brief Erases a key from this set iff it satisfies a given predicate. If
     * no such key exists, the function does nothing. If more than one key
     * matches the given key and satisfies the predicate, only one key will be
     * erased.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.

     * @param k The key to erase.
     * @param predicate A predicate functor.
     * @return True if a key was erased, false otherwise.
     */
    template <typename K, typename Pred>
    bool erase_l(const K &k, Pred predicate) {
        bool retval = false;
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            if (predicate(_table[i].storage.n)) {
                _table[i].release();
                --_size;
                retval = true;
                break;
            }
        }
        _controller.unlock();
        _controller.exit();
        return retval;
    }

    /**
     * @brief Erases all keys from this set that match a given key. If no such
     * key exists, the function does nothing.
     * 
     * @param k The key to erase.
     * @return Number of keys erased.
     */
    template <typename K>
    size_t eraseAll(const K &k) {
        size_t retval = 0;
        size_t h = _hash(k);
        _controller.enter();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            _table[i].release();
            --_size;
            ++retval;
        }
        _controller.exit();
        return retval;
    }

    /**
     * @brief Erases all keys from this set that match a given key. If no such
     * key exists, the function does nothing.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.
     * 
     * @param k The key to erase.
     * @return Number of keys erased.
     */
    template <typename K>
    size_t eraseAll_l(const K &k) {
        size_t retval = 0;
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            _table[i].release();
            --_size;
            ++retval;
        }
        _controller.unlock();
        _controller.exit();
        return retval;
    }

    /**
     * @brief Erases all keys from this set that match a given key and satisfy a
     * given predicate. If no such key exists, the function does nothing.
     * 
     * @param k The key to erase.
     * @param predicate A predicate functor.
     * @return Number of keys erased.
     */
    template <typename K, typename Pred>
    size_t eraseAll(const K &k, Pred predicate) {
        size_t retval = 0;
        size_t h = _hash(k);
        _controller.enter();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            if (predicate(_table[i].storage.n)) {
                _table[i].release();
                --_size;
                ++retval;
            }
        }
        _controller.exit();
        return retval;
    }

    /**
     * @brief Erases all keys from this set that match a given key and satisfy a
     * given predicate. If no such key exists, the function does nothing.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.
     * 
     * @param k The key to erase.
     * @param predicate A predicate functor.
     * @return Number of keys erased.
     */
    template <typename K, typename Pred>
    size_t eraseAll_l(const K &k, Pred predicate) {
        size_t retval = 0;
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            if (predicate(_table[i].storage.n)) {
                _table[i].release();
                --_size;
                ++retval;
            }
        }
        _controller.unlock();
        _controller.exit();
        return retval;
    }

    /**
     * @brief Removes a key from this set and returns it. If the key does not
     * exist, an ElementNotFoundError will be thrown. If more than one key
     * matches the given key, only one key is erased.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The actual removed key.
     */
    template <typename K>
    Key remove(const K &k) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i == __NPOS) {
            _controller.exit();
            throw ElementNotFoundError();
        }
        Key retval = std::move(_table[i].storage.n);
        _table[i].release();
        --_size;
        _controller.exit();
        return retval;
    }

    /**
     * @brief Removes a key from this set and returns it. If the key does not
     * exist, an ElementNotFoundError will be thrown. If more than one key
     * matches the given key, only one key is erased.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The actual removed key.
     */
    template <typename K>
    Key remove_l(const K &k) {
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        size_t i = _findIndex(h, k);
        if (i == __NPOS) {
            _controller.unlock();
            _controller.exit();
            throw ElementNotFoundError();
        }
        Key retval = std::move(_table[i].storage.n);
        _table[i].release();
        --_size;
        _controller.unlock();
        _controller.exit();
        return retval;
    }

    /**
     * @brief Removes a key from this set and returns it iff it satisfies a
     * given predicate. If no such key exists, an ElementNotFoundError will be
     * thrown. If more than one key matches the given key and satisfies the
     * predicate, only one key will be erased.
     * 
     * @param k The key to erase.
     * @param predicate A predicate functor.
     * @throws ElementNotFoundError if the key is not found.
     * @return The value corresponding to the erased key.
     */
    template <typename K, typename Pred>
    Key remove(const K &k, Pred predicate) {
        size_t h = _hash(k);
        _controller.enter();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            if (predicate(_table[i].storage.n)) {
                Key retval = std::move(_table[i].storage.n);
                _table[i].release();
                --_size;
                _controller.exit();
                return retval;
            }
        }
        _controller.exit();
        throw ElementNotFoundError();
    }

    /**
     * @brief Removes a key from this set and returns it iff it satisfies a
     * given predicate. If no such key exists, an ElementNotFoundError will be
     * thrown. If more than one key matches the given key and satisfies the
     * predicate, only one key will be erased.
     * 
     * @param k The key to erase.
     * @param predicate A predicate functor.
     * @throws ElementNotFoundError if the key is not found.
     * @return The value corresponding to the erased key.
     */
    template <typename K, typename Pred>
    Key remove_l(const K &k, Pred predicate) {
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            if (predicate(_table[i].storage.n)) {
                Key retval = std::move(_table[i].storage.n);
                _table[i].release();
                --_size;
                _controller.unlock();
                _controller.exit();
                return retval;
            }
        }
        _controller.unlock();
        _controller.exit();
        throw ElementNotFoundError();
    }

    /**
     * @brief Applies a function to all elements matching a given key.
     * 
     * @param k The key to search for.
     * @param f The function to apply.
     * @return A reference to this container for chaining.
     */
    template <typename K, typename F>
    const HashMultiSet & foreach(const K &k, F f) const {
        size_t h = _hash(k);
        _controller.enter();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            f(_table[i].storage.n);
        }
        _controller.exit();
        return *this;
    }

    /**
     * @brief Applies a function to all elements matching a given key.
     * 
     * @param k The key to search for.
     * @param f The function to apply.
     * @return A reference to this container for chaining.
     */
    template <typename K, typename F>
    HashMultiSet & foreach(const K &k, F f) {
        size_t h = _hash(k);
        _controller.enter();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            f(_table[i].storage.n);
        }
        _controller.exit();
        return *this;
    }

    /**
     * @brief Applies a function to all elements matching a given key.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.
     * 
     * @param k The key to search for.
     * @param f The function to apply.
     * @return A reference to this container for chaining.
     */
    template <typename K, typename F>
    const HashMultiSet & foreach_l(const K &k, F f) const {
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            f(_table[i].storage.n);
        }
        _controller.unlock();
        _controller.exit();
        return *this;
    }

    /**
     * @brief Applies a function to all elements matching a given key.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.
     * 
     * @param k The key to search for.
     * @param f The function to apply.
     * @return A reference to this container for chaining.
     */
    template <typename K, typename F>
    HashMultiSet & foreach_l(const K &k, F f) {
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        auto range = _findRange(h);
        for (size_t i = _findNext(range, h, k); i != __NPOS; i = _findNext(range, h, k)) {
            f(_table[i].storage.n);
        }
        _controller.unlock();
        _controller.exit();
        return *this;
    }
};

}   // namespace parallel

}
