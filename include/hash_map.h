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
 * @brief A hash map supporting O(1) lookup, insert, and delete.
 * 
 * @tparam Key The key type.
 * @tparam Val The value type.
 * @tparam KeyHash A functor to calculate the hash code of the key type. The
 * default functor is Hash<Key>.
 * @tparam KeyEqual A functor to test for key equality. The default functor is
 * std::equal_to<Key>.
*/
template <
    typename Key,
    typename Val,
    typename KeyHash = Hash<Key>,
    typename KeyEqual = std::equal_to<Key>
>
class HashMap
:   protected __HashTable::HashTable<
        Key,
        __HashTable::HashMapNode<Key, Val>,
        KeyHash,
        __HashTable::HashMapNodeKeyEqual<__HashTable::HashMapNode<Key, Val>, Key, KeyEqual>,
        __HashTable::HashTableController,
        size_t
    >,
    public ForwardIterableContainer<HashMap<Key, Val, KeyHash, KeyEqual>>,
    public Serializable
{

    template <typename HashMapType> friend struct HashMapTester;

private:

    using base = typename __HashTable::HashTable<
        Key,
        __HashTable::HashMapNode<Key, Val>,
        KeyHash,
        __HashTable::HashMapNodeKeyEqual<__HashTable::HashMapNode<Key, Val>, Key, KeyEqual>,
        __HashTable::HashTableController,
        size_t
    >;

    using container_base = ForwardIterableContainer<HashMap<Key, Val, KeyHash, KeyEqual>>;

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
     * @brief Construct a new HashMap object.
     */
    HashMap()
    :   base()
    { }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @param initialSize The initial size of the internal hash table. This
     * value could be used to efficiently pre-allocate space, if the number of
     * elements (or an estimate) is known.
     */
    HashMap(size_t initialSize)
    :   base(initialSize)
    { }

    HashMap(const HashMap &rhs)
    :   base(rhs)
    { }

    HashMap(HashMap &&rhs)
    :   base(std::move(rhs))
    { }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @param list An initializer list of objects of type MapNode<Key, Val>.
     */
    HashMap(const std::initializer_list<storage_node> &list)
    :   base(list.size())
    {
        putAll(list);
    }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashMap, Sequence>, int> = 0
    >
    HashMap(const Sequence &seq)
    :   base(seq.size())
    {
        putAll(seq);
    }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashMap, Sequence>, int> = 0
    >
    HashMap(Sequence &&seq)
    :   base(seq.size())
    {
        putAll(std::move(seq));
    }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @param begin A beginning iterator over type MapNode<Key, Val> objects.
     * @param end An end iterator over type MapNode<Key, Val> objects.
     */
    template <typename Begin, typename End>
    HashMap(const Begin &begin, const End &end)
    :   base()
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @param begin A beginning iterator over type MapNode<Key, Val> objects.
     * @param end An end iterator over type MapNode<Key, Val> objects.
     * @param size The number of elements between begin and end.
     */
    template <typename Begin, typename End>
    HashMap(const Begin &begin, const End &end, size_t size)
    :   base(size)
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type. Note that
     * value_type must be a MapNode type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @return A new HashMap object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashMap<
        typename It::value_type::key_type,
        typename It::value_type::value_type
    > create(const It &begin, const EndIt &end) {
        return HashMap<
            typename It::value_type::key_type,
            typename It::value_type::value_type
        >(begin, end);
    }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type. Note that
     * value_type must be a MapNode type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @param size The number of elements between begin and end.
     * @return A new HashMap object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashMap<
        typename It::value_type::key_type,
        typename It::value_type::value_type
    > create(const It &begin, const EndIt &end, size_t size) {
        return HashMap<
            typename It::value_type::key_type,
            typename It::value_type::value_type
        >(begin, end, size);
    }

    ~HashMap() = default;

    HashMap & operator=(const HashMap &rhs) {
        base::operator=(rhs);
        return *this;
    }

    HashMap & operator=(HashMap &&rhs) {
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
    HashMap & clear() {
        base::clear();
        return *this;
    }

    /**
     * @brief Tests whether a mapping for some key exists.
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
     * @brief Retrieves the value corresponding to some key. If the key is not
     * found, an ElementNotFoundError will be thrown.
     * 
     * @param k The key to search for.
     * @throws ElementNotFoundError if the key is not found.
     * @return A copy of the corresponding value.
     */
    template <typename K>
    Val get(const K &k) const {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i != __NPOS) return _table[i].storage.n.v;
        throw ElementNotFoundError();
    }

    /**
     * @brief Retrieves the value corresponding to some key. If the given key is
     * not found in the map, the default value will be returned.
     * 
     * @param k The key to search for.
     * @param defaultValue The default value to return if the key is not found.
     * @return A copy of the actual value corresponding to the given key, or the
     * default value.
     */
    template <typename K>
    Val getOr(const K &k, const Val &defaultValue) const {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i != __NPOS) return _table[i].storage.n.v;
        return defaultValue;
    }

    /**
     * @brief Retrieves the node corresponding to some key. If the key is not
     * found, an ElementNotFoundError will be thrown.
     * 
     * @param k The key to search for.
     * @throws ElementNotFoundError if the key is not found.
     * @return A copy of the corresponding value.
     */
    template <typename K>
    storage_node getNode(const K &k) const {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i != __NPOS) return _table[i].storage.n;
        throw ElementNotFoundError();
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMap & put(const Key &k, const Val &v) {
        size_t h = _hash(k);
        size_t i = _findOrGetFreeIndex(h, k);
        if (_table[i].occupied()) {
            _table[i].storage.n.v = v;
        }
        else {
            _table[i].set(h, { k, v });
            ++_size;
        }
        return *this;
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMap & put(const Key &k, Val &&v) {
        size_t h = _hash(k);
        size_t i = _findOrGetFreeIndex(h, k);
        if (_table[i].occupied()) {
            _table[i].storage.n.v = std::move(v);
        }
        else {
            _table[i].set(h, { k, std::move(v) });
            ++_size;
        }
        return *this;
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMap & put(Key &&k, const Val &v) {
        size_t h = _hash(k);
        size_t i = _findOrGetFreeIndex(h, k);
        if (_table[i].occupied()) {
            _table[i].storage.n.v = v;
        }
        else {
            _table[i].set(h, { std::move(k), v });
            ++_size;
        }
        return *this;
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMap & put(Key &&k, Val &&v) {
        size_t h = _hash(k);
        size_t i = _findOrGetFreeIndex(h, k);
        if (_table[i].occupied()) {
            _table[i].storage.n.v = std::move(v);
        }
        else {
            _table[i].set(h, { std::move(k), std::move(v) });
            ++_size;
        }
        return *this;
    }

    /**
     * @brief Inserts a range of elements.
     * 
     * @param begin A beginning iterator over type MapNode<Key, Val> objects.
     * @param end An end iterator over type MapNode<Key, Val> objects.
     * @return A reference to this container for chaining.
     */
    template <typename Begin, typename End>
    HashMap & putAll(const Begin &begin, const End &end) {
        auto it = begin;
        while (it != end) {
            put((*it).k, (*it).v);
            ++it;
        }
        return *this;
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashMap & putAll(const Sequence &seq) {
        return putAll(seq.begin(), seq.end());
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashMap & putAll(Sequence &&seq) {
        auto it = seq.begin();
        auto end = seq.end();
        while (it != end) {
            put(std::move((*it).k), std::move((*it).v));
            ++it;
        }
        return *this;
    }

    /**
     * @brief Retrieves a reference to the value corresponding to some key. If
     * the mapping does not exist, a new Val type object will be created using
     * the parameterless constructor `Val()`.
     * 
     * @param k The key to search for.
     * @return A refernce to the corresponding value.
     */
    Val & operator[](const Key &k) {
        size_t h = _hash(k);
        size_t i = _findOrGetFreeIndex(h, k);
        if (! _table[i].occupied()) {
            _table[i].set(h, { k, Val() });
        }
        return _table[i].storage.n.v;
    }

    /**
     * @brief Retrieves a reference to the value corresponding to some key. If
     * the mapping does not exist, a new Val type object will be created using
     * the parameterless constructor `Val()`.
     * 
     * @param k The key to search for.
     * @return A refernce to the corresponding value.
     */
    Val & operator[](Key &&k) {
        size_t h = _hash(k);
        size_t i = _findOrGetFreeIndex(h, k);
        if (! _table[i].occupied()) {
            _table[i].set(h, { std::move(k), Val() });
        }
        return _table[i].storage.n.v;
    }

    /**
     * @brief Retrieves a const reference to the value corresponding to some
     * key. If the mapping does not exist, an ElementNotFoundError will be
     * thrown.
     * 
     * @param k The key to search for.
     * @throws ElementNotFoundError if the key is not found.
     * @return A const refernce to the corresponding value.
     */
    const Val & operator[](const Key &k) const {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i == __NPOS) throw ElementNotFoundError();
        return _table[i].storage.n.v;
    }

    /**
     * @brief Erases a key from this map. If the key does not exist, the
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
     * @brief Erases a key from this map iff it satisfies a given predicate. If
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
     * @brief Erases a key from this map and returns the corresponding value. If
     * the key does not exist, an ElementNotFoundError will be thrown.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The value corresponding to the erased key.
     */
    template <typename K>
    Val remove(const K &k) {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i == __NPOS) throw ElementNotFoundError();
        Val retval = std::move(_table[i].storage.n.v);
        _table[i].release();
        --_size;
        return retval;
    }

    /**
     * @brief Removes a node from this map and returns it. If the indicated key
     * does not exist, an ElementNotFoundError will be thrown.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The value corresponding to the erased key.
     */
    template <typename K>
    storage_node removeNode(const K &k) {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i == __NPOS) throw ElementNotFoundError();
        storage_node retval = std::move(_table[i].storage.n);
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
    const HashMap & foreach(const K &k, F f) const {
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
    HashMap & foreach(const K &k, F f) {
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
 * @brief A hash map supporting thread-safe, O(1) lookup, insert, and delete.
 * 
 * @tparam Key The key type.
 * @tparam Val The value type.
 * @tparam KeyHash A functor to calculate the hash code of the key type. The
 * default functor is Hash<Key>.
 * @tparam KeyEqual A functor to test for key equality. The default functor is
 * std::equal_to<Key>.
*/
template <
    typename Key,
    typename Val,
    typename KeyHash = Hash<Key>,
    typename KeyEqual = std::equal_to<Key>
>
class HashMap
:   protected __HashTable::HashTable<
        Key,
        __HashTable::AtomicHashMapNode<Key, Val>,
        KeyHash,
        __HashTable::HashMapNodeKeyEqual<__HashTable::AtomicHashMapNode<Key, Val>, Key, KeyEqual>,
        __HashTable::ConcurrentHashTableController,
        std::atomic_size_t
    >,
    public ForwardIterableContainer<HashMap<Key, Val, KeyHash, KeyEqual>>,
    public Serializable
{

    template <typename HashMapType> friend struct HashMapTester;

private:

    using base = typename __HashTable::HashTable<
        Key,
        __HashTable::AtomicHashMapNode<Key, Val>,
        KeyHash,
        __HashTable::HashMapNodeKeyEqual<__HashTable::AtomicHashMapNode<Key, Val>, Key, KeyEqual>,
        __HashTable::ConcurrentHashTableController,
        std::atomic_size_t
    >;

    using container_base = ForwardIterableContainer<HashMap<Key, Val, KeyHash, KeyEqual>>;

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
     * @brief Construct a new HashMap object.
     */
    HashMap()
    :   base()
    { }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @param initialSize The initial size of the internal hash table. This
     * value could be used to efficiently pre-allocate space, if the number of
     * elements (or an estimate) is known.
     */
    HashMap(size_t initialSize)
    :   base(initialSize)
    { }

    HashMap(const HashMap &rhs)
    :   base(rhs)
    { }

    HashMap(HashMap &&rhs)
    :   base(std::move(rhs))
    { }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @param list An initializer list of objects of type MapNode<Key, Val>.
     */
    HashMap(const std::initializer_list<storage_node> &list)
    :   base(list.size())
    {
        putAll(list);
    }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashMap, Sequence>, int> = 0
    >
    HashMap(const Sequence &seq)
    :   base(seq.size())
    {
        putAll(seq);
    }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashMap, Sequence>, int> = 0
    >
    HashMap(Sequence &&seq)
    :   base(seq.size())
    {
        putAll(std::move(seq));
    }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @param begin A beginning iterator over type MapNode<Key, Val> objects.
     * @param end An end iterator over type MapNode<Key, Val> objects.
     */
    template <typename Begin, typename End>
    HashMap(const Begin &begin, const End &end)
    :   base()
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @param begin A beginning iterator over type MapNode<Key, Val> objects.
     * @param end An end iterator over type MapNode<Key, Val> objects.
     * @param size The number of elements between begin and end.
     */
    template <typename Begin, typename End>
    HashMap(const Begin &begin, const End &end, size_t size)
    :   base(size)
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type. Note that
     * value_type must be a MapNode type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @return A new HashMap object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashMap<
        typename It::value_type::key_type,
        typename It::value_type::value_type
    > create(const It &begin, const EndIt &end) {
        return HashMap<
            typename It::value_type::key_type,
            typename It::value_type::value_type
        >(begin, end);
    }

    /**
     * @brief Construct a new HashMap object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type. Note that
     * value_type must be a MapNode type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @param size The number of elements between begin and end.
     * @return A new HashMap object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashMap<
        typename It::value_type::key_type,
        typename It::value_type::value_type
    > create(const It &begin, const EndIt &end, size_t size) {
        return HashMap<
            typename It::value_type::key_type,
            typename It::value_type::value_type
        >(begin, end, size);
    }

    ~HashMap() = default;

    HashMap & operator=(const HashMap &rhs) {
        base::operator=(rhs);
        return *this;
    }

    HashMap & operator=(HashMap &&rhs) {
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
    HashMap & clear() {
        base::clear();
        return *this;
    }

    /**
     * @brief Tests whether a mapping for some key exists.
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
     * @brief Retrieves the value corresponding to some key. If the key is not
     * found, an ElementNotFoundError will be thrown.
     * 
     * @param k The key to search for.
     * @throws ElementNotFoundError if the key is not found.
     * @return A copy of the corresponding value.
     */
    template <typename K>
    Val get(const K &k) const {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            Val retval = _table[i].storage.n.v;
            _controller.exit();
            return retval;
        }
        _controller.exit();
        throw ElementNotFoundError();
    }

    /**
     * @brief Retrieves the value corresponding to some key. If the given key is
     * not found in the map, the default value will be returned.
     * 
     * @param k The key to search for.
     * @param defaultValue The default value to return if the key is not found.
     * @return A copy of the actual value corresponding to the given key, or the
     * default value.
     */
    template <typename K>
    Val getOr(const K &k, const Val &defaultValue) const {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            Val retval = _table[i].storage.n.v;
            _controller.exit();
            return retval;
        }
        _controller.exit();
        return defaultValue;
    }

    /**
     * @brief Retrieves the node corresponding to some key. If the key is not
     * found, an ElementNotFoundError will be thrown.
     * 
     * @param k The key to search for.
     * @throws ElementNotFoundError if the key is not found.
     * @return A copy of the corresponding value.
     */
    template <typename K>
    storage_node getNode(const K &k) const {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            storage_node retval = _table[i].storage.n;
            _controller.exit();
            return retval;
        }
        _controller.exit();
        throw ElementNotFoundError();
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMap & put(const Key &k, const Val &v) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findOrGetFreeIndex(h, k);
        if (_table[i].occupied()) {
            _table[i].storage.n.v = v;
        }
        else {
            _table[i].set(h, { k, v });
            ++_size;
        }
        _controller.exit();
        return *this;
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMap & put(const Key &k, Val &&v) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findOrGetFreeIndex(h, k);
        if (_table[i].occupied()) {
            _table[i].storage.n.v = std::move(v);
        }
        else {
            _table[i].set(h, { k, std::move(v) });
            ++_size;
        }
        _controller.exit();
        return *this;
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMap & put(Key &&k, const Val &v) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findOrGetFreeIndex(h, k);
        if (_table[i].occupied()) {
            _table[i].storage.n.v = v;
        }
        else {
            _table[i].set(h, { std::move(k), v });
            ++_size;
        }
        _controller.exit();
        return *this;
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMap & put(Key &&k, Val &&v) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findOrGetFreeIndex(h, k);
        if (_table[i].occupied()) {
            _table[i].storage.n.v = std::move(v);
        }
        else {
            _table[i].set(h, { std::move(k), std::move(v) });
            ++_size;
        }
        _controller.exit();
        return *this;
    }

    /**
     * @brief Inserts a range of elements.
     * 
     * @param begin A beginning iterator over type MapNode<Key, Val> objects.
     * @param end An end iterator over type MapNode<Key, Val> objects.
     * @return A reference to this container for chaining.
     */
    template <typename Begin, typename End>
    HashMap & putAll(const Begin &begin, const End &end) {
        auto it = begin;
        while (it != end) {
            put((*it).k, (*it).v);
            ++it;
        }
        return *this;
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashMap & putAll(const Sequence &seq) {
        return putAll(seq.begin(), seq.end());
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashMap & putAll(Sequence &&seq) {
        auto it = seq.begin();
        auto end = seq.end();
        while (it != end) {
            put(std::move((*it).k), std::move((*it).v));
            ++it;
        }
        return *this;
    }

    /**
     * @brief Retrieves a reference to the value corresponding to some key. If
     * the mapping does not exist, a new Val type object will be created using
     * the parameterless constructor `Val()`.
     * Note: this function is not thread-safe.
     * 
     * @param k The key to search for.
     * @return A refernce to the corresponding value.
     */
    Val & operator[](const Key &k) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findOrGetFreeIndex(h, k);
        if (! _table[i].occupied()) {
            _table[i].set(h, { k, Val() });
        }
        _controller.exit();
        return _table[i].storage.n.v;
    }

    /**
     * @brief Retrieves a reference to the value corresponding to some key. If
     * the mapping does not exist, a new Val type object will be created using
     * the parameterless constructor `Val()`.
     * Note: this function is not thread-safe.
     * 
     * @param k The key to search for.
     * @return A refernce to the corresponding value.
     */
    Val & operator[](Key &&k) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findOrGetFreeIndex(h, k);
        if (! _table[i].occupied()) {
            _table[i].set(h, { std::move(k), Val() });
        }
        _controller.exit();
        return _table[i].storage.n.v;
    }

    /**
     * @brief Retrieves a const reference to the value corresponding to some
     * key. If the mapping does not exist, an ElementNotFoundError will be
     * thrown.
     * Note: this function is not thread-safe.
     * 
     * @param k The key to search for.
     * @throws ElementNotFoundError if the key is not found.
     * @return A const refernce to the corresponding value.
     */
    const Val & operator[](const Key &k) const {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i == __NPOS) throw ElementNotFoundError();
        return _table[i].storage.n.v;
    }

    /**
     * @brief Erases a key from this map. If the key does not exist, the
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
     * @brief Erases a key from this map. If the key does not exist, the
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
     * @brief Erases a key from this map iff it satisfies a given predicate. If
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
     * @brief Erases a key from this map iff it satisfies a given predicate. If
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
     * @brief Erases a key from this map and returns the corresponding value. If
     * the key does not exist, an ElementNotFoundError will be thrown.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The value corresponding to the erased key.
     */
    template <typename K>
    Val remove(const K &k) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i == __NPOS) {
            _controller.exit();
            throw ElementNotFoundError();
        }
        Val retval = std::move(_table[i].storage.n.v);
        _table[i].release();
        --_size;
        _controller.exit();
        return retval;
    }

    /**
     * @brief Erases a key from this map and returns the corresponding value. If
     * the key does not exist, an ElementNotFoundError will be thrown.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The value corresponding to the erased key.
     */
    template <typename K>
    Val remove_l(const K &k) {
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        size_t i = _findIndex(h, k);
        if (i == __NPOS) {
            _controller.unlock();
            _controller.exit();
            throw ElementNotFoundError();
        }
        Val retval = std::move(_table[i].storage.n.v);
        _table[i].release();
        --_size;
        _controller.unlock();
        _controller.exit();
        return retval;
    }

    /**
     * @brief Removes a node from this map and returns it. If the indicated key
     * does not exist, an ElementNotFoundError will be thrown.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The value corresponding to the erased key.
     */
    template <typename K>
    storage_node removeNode(const K &k) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i == __NPOS) {
            _controller.exit();
            throw ElementNotFoundError();
        }
        storage_node retval = std::move(_table[i].storage.n);
        _table[i].release();
        --_size;
        _controller.exit();
        return retval;
    }

    /**
     * @brief Removes a node from this map and returns it. If the indicated key
     * does not exist, an ElementNotFoundError will be thrown.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The value corresponding to the erased key.
     */
    template <typename K>
    storage_node removeNode_l(const K &k) {
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        size_t i = _findIndex(h, k);
        if (i == __NPOS) {
            _controller.unlock();
            _controller.exit();
            throw ElementNotFoundError();
        }
        storage_node retval = std::move(_table[i].storage.n);
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
    const HashMap & foreach(const K &k, F f) const {
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
    HashMap & foreach(const K &k, F f) {
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
    const HashMap & foreach_l(const K &k, F f) const {
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
    HashMap & foreach_l(const K &k, F f) {
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
 * @brief A hash multi-map supporting O(1) lookup, insert, and delete.
 * 
 * @tparam Key The key type.
 * @tparam Val The value type.
 * @tparam KeyHash A functor to calculate the hash code of the key type. The
 * default functor is Hash<Key>.
 * @tparam KeyEqual A functor to test for key equality. The default functor is
 * std::equal_to<Key>.
*/
template <
    typename Key,
    typename Val,
    typename KeyHash = Hash<Key>,
    typename KeyEqual = std::equal_to<Key>
>
class HashMultiMap
:   protected __HashTable::HashTable<
        Key,
        __HashTable::HashMapNode<Key, Val>,
        KeyHash,
        __HashTable::HashMapNodeKeyEqual<__HashTable::HashMapNode<Key, Val>, Key, KeyEqual>,
        __HashTable::HashTableController,
        size_t
    >,
    public ForwardIterableContainer<HashMultiMap<Key, Val, KeyHash, KeyEqual>>,
    public Serializable
{

    template <typename HashMapType> friend struct HashMapTester;

private:

    using base = typename __HashTable::HashTable<
        Key,
        __HashTable::HashMapNode<Key, Val>,
        KeyHash,
        __HashTable::HashMapNodeKeyEqual<__HashTable::HashMapNode<Key, Val>, Key, KeyEqual>,
        __HashTable::HashTableController,
        size_t
    >;

    using container_base = ForwardIterableContainer<HashMultiMap<Key, Val, KeyHash, KeyEqual>>;

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
     * @brief Construct a new HashMultiMap object
     */
    HashMultiMap()
    :   base()
    { }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @param initialSize The initial size of the internal hash table. This
     * value could be used to efficiently pre-allocate space, if the number of
     * elements (or an estimate) is known.
     */
    HashMultiMap(size_t initialSize)
    :   base(initialSize)
    { }

    HashMultiMap(const HashMultiMap &rhs)
    :   base(rhs)
    { }

    HashMultiMap(HashMultiMap &&rhs)
    :   base(std::move(rhs))
    { }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @param list An initializer list of objects of type MapNode<Key, Val>.
     */
    HashMultiMap(const std::initializer_list<storage_node> &list)
    :   base(list.size())
    {
        putAll(list);
    }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashMultiMap, Sequence>, int> = 0
    >
    HashMultiMap(const Sequence &seq)
    :   base(seq.size())
    {
        putAll(seq);
    }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashMultiMap, Sequence>, int> = 0
    >
    HashMultiMap(Sequence &&seq)
    :   base(seq.size())
    {
        putAll(std::move(seq));
    }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @param begin A beginning iterator over type MapNode<Key, Val> objects.
     * @param end An end iterator over type MapNode<Key, Val> objects.
     */
    template <typename Begin, typename End>
    HashMultiMap(const Begin &begin, const End &end)
    :   base()
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @param begin A beginning iterator over type MapNode<Key, Val> objects.
     * @param end An end iterator over type MapNode<Key, Val> objects.
     * @param size The number of elements between begin and end.
     */
    template <typename Begin, typename End>
    HashMultiMap(const Begin &begin, const End &end, size_t size)
    :   base(size)
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type. Note that
     * value_type must be a MapNode type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @return A new HashMultiMap object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashMultiMap<
        typename It::value_type::key_type,
        typename It::value_type::value_type
    > create(const It &begin, const EndIt &end) {
        return HashMultiMap<
            typename It::value_type::key_type,
            typename It::value_type::value_type
        >(begin, end);
    }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type. Note that
     * value_type must be a MapNode type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @param size The number of elements between begin and end.
     * @return A new HashMultiMap object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashMultiMap<
        typename It::value_type::key_type,
        typename It::value_type::value_type
    > create(const It &begin, const EndIt &end, size_t size) {
        return HashMultiMap<
            typename It::value_type::key_type,
            typename It::value_type::value_type
        >(begin, end, size);
    }

    ~HashMultiMap() = default;

    HashMultiMap & operator=(const HashMultiMap &rhs) {
        base::operator=(rhs);
        return *this;
    }

    HashMultiMap & operator=(HashMultiMap &&rhs) {
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
    HashMultiMap & clear() {
        base::clear();
        return *this;
    }

    /**
     * @brief Tests whether a mapping for some key exists.
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
     * @brief Retrieves a value corresponding to some key. If the key is not
     * found, an ElementNotFoundError will be thrown.
     * 
     * @param k The key to search for.
     * @throws ElementNotFoundError if the key is not found.
     * @return A copy of the corresponding value.
     */
    template <typename K>
    Val get(const K &k) const {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i != __NPOS) return _table[i].storage.n.v;
        throw ElementNotFoundError();
    }

    /**
     * @brief Retrieves a value corresponding to some key. If the given key is
     * not found in the map, the default value will be returned.
     * 
     * @param k The key to search for.
     * @param defaultValue The default value to return if the key is not found.
     * @return A copy of the actual value corresponding to the given key, or the
     * default value.
     */
    template <typename K>
    Val getOr(const K &k, const Val &defaultValue) const {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i != __NPOS) return _table[i].storage.n.v;
        return defaultValue;
    }

    /**
     * @brief Retrieves a node corresponding to some key. If the key is not
     * found, an ElementNotFoundError will be thrown.
     * 
     * @param k The key to search for.
     * @throws ElementNotFoundError if the key is not found.
     * @return A copy of the corresponding value.
     */
    template <typename K>
    storage_node getNode(const K &k) const {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i != __NPOS) return _table[i].storage.n;
        throw ElementNotFoundError();
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMultiMap & put(const Key &k, const Val &v) {
        size_t h = _hash(k);
        size_t i = _getFreeIndex(h);
        _table[i].set(h, { k, v });
        ++_size;
        return *this;
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMultiMap & put(const Key &k, Val &&v) {
        size_t h = _hash(k);
        size_t i = _getFreeIndex(h);
        _table[i].set(h, { k, std::move(v) });
        ++_size;
        return *this;
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMultiMap & put(Key &&k, const Val &v) {
        size_t h = _hash(k);
        size_t i = _getFreeIndex(h);
        _table[i].set(h, { std::move(k), v });
        ++_size;
        return *this;
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMultiMap & put(Key &&k, Val &&v) {
        size_t h = _hash(k);
        size_t i = _getFreeIndex(h);
        _table[i].set(h, { std::move(k), std::move(v) });
        ++_size;
        return *this;
    }

    /**
     * @brief Inserts a range of elements.
     * 
     * @param begin A beginning iterator over type MapNode<Key, Val> objects.
     * @param end An end iterator over type MapNode<Key, Val> objects.
     * @return A reference to this container for chaining.
     */
    template <typename Begin, typename End>
    HashMultiMap & putAll(const Begin &begin, const End &end) {
        auto it = begin;
        while (it != end) {
            put((*it).k, (*it).v);
            ++it;
        }
        return *this;
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashMultiMap & putAll(const Sequence &seq) {
        return putAll(seq.begin(), seq.end());
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashMultiMap & putAll(Sequence &&seq) {
        auto it = seq.begin();
        auto end = seq.end();
        while (it != end) {
            put(std::move((*it).k), std::move((*it).v));
            ++it;
        }
        return *this;
    }

    /**
     * @brief Erases a key from this map. If the key does not exist, the
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
     * @brief Erases a key from this map iff it satisfies a given predicate. If
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
     * @brief Erases a key from this map and returns the corresponding value. If
     * the key does not exist, an ElementNotFoundError will be thrown. If more
     * than one key matches the given key, only one key is erased.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The value corresponding to the erased key.
     */
    template <typename K>
    Val remove(const K &k) {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i == __NPOS) throw ElementNotFoundError();
        Val retval = std::move(_table[i].storage.n.v);
        _table[i].release();
        --_size;
        return retval;
    }

    /**
     * @brief Removes a node from this map and returns it. If the indicated key
     * does not exist, an ElementNotFoundError will be thrown. If more than one
     * key matches the given key, only one node is removed.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The value corresponding to the erased key.
     */
    template <typename K>
    storage_node removeNode(const K &k) {
        size_t h = _hash(k);
        size_t i = _findIndex(h, k);
        if (i == __NPOS) throw ElementNotFoundError();
        storage_node retval = std::move(_table[i].storage.n);
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
    const HashMultiMap & foreach(const K &k, F f) const {
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
    HashMultiMap & foreach(const K &k, F f) {
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
 * @brief A hash multi-map supporting thread-safe, O(1) lookup, insert, and
 * delete.
 * 
 * @tparam Key The key type.
 * @tparam Val The value type.
 * @tparam KeyHash A functor to calculate the hash code of the key type. The
 * default functor is Hash<Key>.
 * @tparam KeyEqual A functor to test for key equality. The default functor is
 * std::equal_to<Key>.
*/
template <
    typename Key,
    typename Val,
    typename KeyHash = Hash<Key>,
    typename KeyEqual = std::equal_to<Key>
>
class HashMultiMap
:   protected __HashTable::HashTable<
        Key,
        __HashTable::AtomicHashMapNode<Key, Val>,
        KeyHash,
        __HashTable::HashMapNodeKeyEqual<__HashTable::AtomicHashMapNode<Key, Val>, Key, KeyEqual>,
        __HashTable::ConcurrentHashTableController,
        std::atomic_size_t
    >,
    public ForwardIterableContainer<HashMultiMap<Key, Val, KeyHash, KeyEqual>>,
    public Serializable
{

    template <typename HashMapType> friend struct HashMapTester;

private:

    using base = typename __HashTable::HashTable<
        Key,
        __HashTable::AtomicHashMapNode<Key, Val>,
        KeyHash,
        __HashTable::HashMapNodeKeyEqual<__HashTable::AtomicHashMapNode<Key, Val>, Key, KeyEqual>,
        __HashTable::ConcurrentHashTableController,
        std::atomic_size_t
    >;

    using container_base = ForwardIterableContainer<HashMultiMap<Key, Val, KeyHash, KeyEqual>>;

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
     * @brief Construct a new HashMultiMap object
     */
    HashMultiMap()
    :   base()
    { }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @param initialSize The initial size of the internal hash table. This
     * value could be used to efficiently pre-allocate space, if the number of
     * elements (or an estimate) is known.
     */
    HashMultiMap(size_t initialSize)
    :   base(initialSize)
    { }

    HashMultiMap(const HashMultiMap &rhs)
    :   base(rhs)
    { }

    HashMultiMap(HashMultiMap &&rhs)
    :   base(std::move(rhs))
    { }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @param list An initializer list of objects of type MapNode<Key, Val>.
     */
    HashMultiMap(const std::initializer_list<storage_node> &list)
    :   base(list.size())
    {
        putAll(list);
    }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashMultiMap, Sequence>, int> = 0
    >
    HashMultiMap(const Sequence &seq)
    :   base(seq.size())
    {
        putAll(seq);
    }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     */
    template <
        typename Sequence,
        std::enable_if_t<! std::is_same_v<HashMultiMap, Sequence>, int> = 0
    >
    HashMultiMap(Sequence &&seq)
    :   base(seq.size())
    {
        putAll(std::move(seq));
    }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @param begin A beginning iterator over type MapNode<Key, Val> objects.
     * @param end An end iterator over type MapNode<Key, Val> objects.
     */
    template <typename Begin, typename End>
    HashMultiMap(const Begin &begin, const End &end)
    :   base()
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @param begin A beginning iterator over type MapNode<Key, Val> objects.
     * @param end An end iterator over type MapNode<Key, Val> objects.
     * @param size The number of elements between begin and end.
     */
    template <typename Begin, typename End>
    HashMultiMap(const Begin &begin, const End &end, size_t size)
    :   base(size)
    {
        putAll(begin, end);
    }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type. Note that
     * value_type must be a MapNode type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @return A new HashMultiMap object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashMultiMap<
        typename It::value_type::key_type,
        typename It::value_type::value_type
    > create(const It &begin, const EndIt &end) {
        return HashMultiMap<
            typename It::value_type::key_type,
            typename It::value_type::value_type
        >(begin, end);
    }

    /**
     * @brief Construct a new HashMultiMap object.
     * 
     * @tparam It A beginning iterator type that defines the typename
     * `value_type` which will determine the resulting type. Note that
     * value_type must be a MapNode type.
     * @tparam EndIt An end iterator type.
     * @param begin The beginning iterator.
     * @param end The end iterator.
     * @param size The number of elements between begin and end.
     * @return A new HashMultiMap object of type It::value_type objects.
     */
    template <typename It, typename EndIt>
    static HashMultiMap<
        typename It::value_type::key_type,
        typename It::value_type::value_type
    > create(const It &begin, const EndIt &end, size_t size) {
        return HashMultiMap<
            typename It::value_type::key_type,
            typename It::value_type::value_type
        >(begin, end, size);
    }

    ~HashMultiMap() = default;

    HashMultiMap & operator=(const HashMultiMap &rhs) {
        base::operator=(rhs);
        return *this;
    }

    HashMultiMap & operator=(HashMultiMap &&rhs) {
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
    HashMultiMap & clear() {
        base::clear();
        return *this;
    }

    /**
     * @brief Tests whether a mapping for some key exists.
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
     * @brief Retrieves a value corresponding to some key. If the key is not
     * found, an ElementNotFoundError will be thrown.
     * 
     * @param k The key to search for.
     * @throws ElementNotFoundError if the key is not found.
     * @return A copy of the corresponding value.
     */
    template <typename K>
    Val get(const K &k) const {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            Val retval = _table[i].storage.n.v;
            _controller.exit();
            return retval;
        }
        _controller.exit();
        throw ElementNotFoundError();
    }

    /**
     * @brief Retrieves a value corresponding to some key. If the given key is
     * not found in the map, the default value will be returned.
     * 
     * @param k The key to search for.
     * @param defaultValue The default value to return if the key is not found.
     * @return A copy of the actual value corresponding to the given key, or the
     * default value.
     */
    template <typename K>
    Val getOr(const K &k, const Val &defaultValue) const {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            Val retval = _table[i].storage.n.v;
            _controller.exit();
            return retval;
        }
        _controller.exit();
        return defaultValue;
    }

    /**
     * @brief Retrieves the node corresponding to some key. If the key is not
     * found, an ElementNotFoundError will be thrown.
     * 
     * @param k The key to search for.
     * @throws ElementNotFoundError if the key is not found.
     * @return A copy of the corresponding value.
     */
    template <typename K>
    storage_node getNode(const K &k) const {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            storage_node retval = _table[i].storage.n;
            _controller.exit();
            return retval;
        }
        _controller.exit();
        throw ElementNotFoundError();
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMultiMap & put(const Key &k, const Val &v) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _getFreeIndex(h);
        _table[i].set(h, { k, v });
        ++_size;
        _controller.exit();
        return *this;
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMultiMap & put(const Key &k, Val &&v) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _getFreeIndex(h);
        _table[i].set(h, { k, std::move(v) });
        ++_size;
        _controller.exit();
        return *this;
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMultiMap & put(Key &&k, const Val &v) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _getFreeIndex(h);
        _table[i].set(h, { std::move(k), v });
        ++_size;
        _controller.exit();
        return *this;
    }

    /**
     * @brief Inserts a new mapping.
     * 
     * @param k Key.
     * @param v Value.
     * @return A reference to this container for chaining.
     */
    HashMultiMap & put(Key &&k, Val &&v) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _getFreeIndex(h);
        _table[i].set(h, { std::move(k), std::move(v) });
        ++_size;
        _controller.exit();
        return *this;
    }

    /**
     * @brief Inserts a range of elements.
     * 
     * @param begin A beginning iterator over type MapNode<Key, Val> objects.
     * @param end An end iterator over type MapNode<Key, Val> objects.
     * @return A reference to this container for chaining.
     */
    template <typename Begin, typename End>
    HashMultiMap & putAll(const Begin &begin, const End &end) {
        auto it = begin;
        while (it != end) {
            put((*it).k, (*it).v);
            ++it;
        }
        return *this;
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashMultiMap & putAll(const Sequence &seq) {
        return putAll(seq.begin(), seq.end());
    }

    /**
     * @brief Inserts all elements of an iterable container.
     * 
     * @param seq An iterable container of type MapNode<Key, Val> objects.
     * @return A reference to this container for chaining.
     */
    template <typename Sequence>
    HashMultiMap & putAll(Sequence &&seq) {
        auto it = seq.begin();
        auto end = seq.end();
        while (it != end) {
            put(std::move((*it).k), std::move((*it).v));
            ++it;
        }
        return *this;
    }

    /**
     * @brief Erases a key from this map. If the key does not exist, the
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
     * @brief Erases a key from this map. If the key does not exist, the
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
     * @brief Erases a key from this map iff it satisfies a given predicate. If
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
     * @brief Erases a key from this map iff it satisfies a given predicate. If
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
     * @brief Erases a key from this map and returns the corresponding value. If
     * the key does not exist, an ElementNotFoundError will be thrown. If more
     * than one key matches the given key, only one key is erased.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The value corresponding to the erased key.
     */
    template <typename K>
    Val remove(const K &k) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            Val retval = std::move(_table[i].storage.n.v);
            _table[i].release();
            --_size;
            _controller.exit();
            return retval;
        }
        _controller.exit();
        throw ElementNotFoundError();
    }

    /**
     * @brief Erases a key from this map and returns the corresponding value. If
     * the key does not exist, an ElementNotFoundError will be thrown. If more
     * than one key matches the given key, only one key is erased.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The value corresponding to the erased key.
     */
    template <typename K>
    Val remove_l(const K &k) {
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            Val retval = std::move(_table[i].storage.n.v);
            _table[i].release();
            --_size;
            _controller.unlock();
            _controller.exit();
            return retval;
        }
        _controller.unlock();
        _controller.exit();
        throw ElementNotFoundError();
    }

    /**
     * @brief Removes a node from this map and returns it. If the indicated key
     * does not exist, an ElementNotFoundError will be thrown. If more than one
     * key matches the given key, only one node is removed.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The value corresponding to the erased key.
     */
    template <typename K>
    storage_node removeNode(const K &k) {
        size_t h = _hash(k);
        _controller.enter();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            storage_node retval = std::move(_table[i].storage.n);
            _table[i].release();
            --_size;
            _controller.exit();
            return retval;
        }
        _controller.exit();
        throw ElementNotFoundError();
    }

    /**
     * @brief Removes a node from this map and returns it. If the indicated key
     * does not exist, an ElementNotFoundError will be thrown. If more than one
     * key matches the given key, only one node is removed.
     * Note: This variant obtains a lock to ensure that no other concurrent
     * operation may be performed on this container.
     * 
     * @param k The key to erase.
     * @throws ElementNotFoundError if the key is not found.
     * @return The value corresponding to the erased key.
     */
    template <typename K>
    storage_node removeNode_l(const K &k) {
        size_t h = _hash(k);
        _controller.enter();
        _controller.lock();
        size_t i = _findIndex(h, k);
        if (i != __NPOS) {
            storage_node retval = std::move(_table[i].storage.n);
            _table[i].release();
            --_size;
            _controller.unlock();
            _controller.exit();
            return retval;
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
    const HashMultiMap & foreach(const K &k, F f) const {
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
    HashMultiMap & foreach(const K &k, F f) {
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
    const HashMultiMap & foreach_l(const K &k, F f) const {
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
    HashMultiMap & foreach_l(const K &k, F f) {
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

}   // namespace spl
