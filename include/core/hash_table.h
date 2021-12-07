/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <atomic>
#include <mutex>
#include <cstring>
#include <cstdlib>
#include <iterator.h>
#include <serialization.h>
#include <type_traits>
#include <exception.h>

namespace spl {

namespace __HashTable {
    template <
        typename K,
        typename node,
        typename KeyHash,
        typename NodeKeyEqual,
        typename Controller,
        typename size_type
    >
    class HashTable;
}

/**
 * @brief A node for map containers.
 * 
 * @tparam Key The key type.
 * @tparam Val The value type.
 */
template <typename Key, typename Val>
class MapNode {

    template <
        typename K,
        typename node,
        typename KeyHash,
        typename NodeKeyEqual,
        typename Controller,
        typename size_type
    >
    friend class __HashTable::HashTable;

private:

    void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const {
        serializer << k << v;
    }

    void readObject(InputStreamSerializer &serializer, SerializationLevel level) {
        serializer >> *const_cast<Key *>(&k) >> v;
    }

public:

    using key_type = Key;
    using value_type = Val;

    const Key k;
    Val v;

    MapNode()
    :   k(),
        v()
    { }

    MapNode(const Key &k, const Val &v)
    :   k(k),
        v(v)
    { }

    MapNode(const Key &k, Val &&v)
    :   k(k),
        v(std::move(v))
    { }

    MapNode(Key &&k, const Val &v)
    :   k(std::move(k)),
        v(v)
    { }

    MapNode(Key &&k, Val &&v)
    :   k(std::move(k)),
        v(std::move(v))
    { }

    MapNode(const MapNode &) = default;

    MapNode(MapNode &&) = default;

    ~MapNode() = default;

    MapNode & operator=(const MapNode &rhs) {
        *const_cast<Key *>(&k) = rhs.k;
        v = rhs.v;
        return *this;
    }

    MapNode & operator=(MapNode &&rhs) {
        *const_cast<Key *>(&k) = std::move(rhs.k);
        v = std::move(rhs.v);
        return *this;
    }
};

template <typename Key, typename Val>
struct SupportsTrivialSerialization_t<MapNode<Key, Val>> {
    static constexpr bool value = SupportsTrivialSerialization<Key> && SupportsTrivialSerialization<Val>;
};

template <typename Key, typename Val>
struct SupportsCustomSerialization_t<MapNode<Key, Val>> {
    static constexpr bool value = SupportsCustomSerialization<Key> || SupportsCustomSerialization<Val>;
};

namespace __HashTable {

constexpr uint8_t UNOCCUPIED = 0;
constexpr uint8_t TENTATIVELY_OCCUPIED = 1;
constexpr uint8_t OCCUPIED = 2;

template <typename Node>
struct HashTableNode {

    using storage_type = Node;

    size_t h;
    uint8_t status;
    union storage {
        alignas(storage_type) unsigned char buf[sizeof(storage_type)];
        storage_type n;

        storage() {}
        ~storage() {}
    } storage;

    HashTableNode()
    :   h(0)
    { }

    HashTableNode(const HashTableNode &) = delete;

    HashTableNode(HashTableNode &&) = delete;

    ~HashTableNode() = default;

    HashTableNode & operator=(const HashTableNode &) = delete;

    HashTableNode & operator=(HashTableNode &&) = delete;

    void * operator new[](std::size_t count) {
        return calloc(count, 1);
    }

    void * operator new[](std::size_t count, std::align_val_t al) {
        return calloc(count, 1);
    }

    void operator delete[](void* ptr) {
        free(ptr);
    }

    void operator delete[](void* ptr, std::align_val_t al) {
        free(ptr);
    }

    void operator delete[](void* ptr, std::size_t sz) {
        free(ptr);
    }

    void operator delete[](void* ptr, std::size_t sz, std::align_val_t al) {
        free(ptr);
    }

    void set(size_t hash, const storage_type &node) {
        new (&storage) storage_type(node);
        h = hash;
        status = OCCUPIED;
    }

    void set(size_t hash, storage_type &&node) {
        new (&storage) storage_type(std::move(node));
        h = hash;
        status = OCCUPIED;
    }

    bool occupyIfFree() {
        if (status == UNOCCUPIED) {
            status = TENTATIVELY_OCCUPIED;
            return true;
        }
        else {
            return false;
        }
    }

    void release() {
        status = TENTATIVELY_OCCUPIED;
        storage.n.~storage_type();
        status = UNOCCUPIED;
    }

    bool occupied() {
        return status == OCCUPIED;
    }
};

template <typename Node>
struct AtomicHashTableNode {

    using storage_type = Node;

    size_t h;
    std::atomic<uint8_t> status;
    union storage {
        alignas(storage_type) unsigned char buf[sizeof(storage_type)];
        storage_type n;

        storage() {}
        ~storage() {}
    } storage;

    AtomicHashTableNode()
    :   h(0)
    { }

    AtomicHashTableNode(const AtomicHashTableNode &) = delete;

    AtomicHashTableNode(AtomicHashTableNode &&) = delete;

    ~AtomicHashTableNode() = default;

    AtomicHashTableNode & operator=(const AtomicHashTableNode &) = delete;

    AtomicHashTableNode & operator=(AtomicHashTableNode &&rhs) = delete;

    void * operator new[](std::size_t count) {
        // TODO: check alignment
        return calloc(count, 1);
    }

    void * operator new[](std::size_t count, std::align_val_t al) {
        return calloc(count, 1);
    }

    void operator delete[](void* ptr) {
        free(ptr);
    }

    void operator delete[](void* ptr, std::align_val_t al) {
        free(ptr);
    }

    void operator delete[](void* ptr, std::size_t sz) {
        free(ptr);
    }

    void operator delete[](void* ptr, std::size_t sz, std::align_val_t al) {
        free(ptr);
    }

    void set(size_t hash, const storage_type &node) {
        new (&storage) storage_type(node);
        h = hash;
        status.store(OCCUPIED, std::memory_order_release);
    }

    void set(size_t hash, storage_type &&node) {
        new (&storage) storage_type(std::move(node));
        h = hash;
        status.store(OCCUPIED, std::memory_order_release);
    }

    bool occupyIfFree() {
        uint8_t unoccupied = UNOCCUPIED;
        return status.compare_exchange_weak(
            unoccupied,
            TENTATIVELY_OCCUPIED,
            std::memory_order_release,
            std::memory_order_relaxed
        );
    }

    void release() {
        status.store(TENTATIVELY_OCCUPIED, std::memory_order_release);
        storage.n.~storage_type();
        status.store(UNOCCUPIED, std::memory_order_release);
    }

    bool occupied() {
        return status.load(std::memory_order_relaxed) == OCCUPIED;
    }
};

template <typename Key, typename Val>
using HashMapNode = HashTableNode<MapNode<Key, Val>>;

template <typename Key, typename Val>
using AtomicHashMapNode = AtomicHashTableNode<MapNode<Key, Val>>;

template <typename Key>
using HashSetNode = HashTableNode<Key>;

template <typename Key>
using AtomicHashSetNode = AtomicHashTableNode<Key>;

template <typename Node, typename Key, typename KeyEqual> 
struct HashMapNodeKeyEqual {
    KeyEqual _eq;
    bool operator()(const Node &n, const Key &k) const {
        return _eq(n.storage.n.k, k);
    }
};

template <typename Node, typename Key, typename KeyEqual> 
struct HashSetNodeKeyEqual {
    KeyEqual _eq;
    bool operator()(const Node &n, const Key &k) const {
        return _eq(n.storage.n, k);
    }
};

struct HashRange {
    size_t pos;
    size_t end;
    size_t size;

    bool nonEmpty() {
        return pos != end;
    }

    void next() {
        pos = (pos + 1) % size;
    }
};

struct HashTableController {

    static constexpr size_t LINEAR_INCREMENT_THRESHOLD = 100000000lu;
    static constexpr size_t BUCKET_SEARCH = 16;               // max number of buckets to search in for a key
    static constexpr size_t INITIAL_BUCKET_SIZE = 1;          // initial size of a bucket with matching hash values

    size_t bucketSize = 0; // bucket size
    size_t nBuckets = 0;   // number of buckets
    size_t tableSize = 0;  // size of the table

    void init(size_t sz) {
        bucketSize = INITIAL_BUCKET_SIZE;
        nBuckets = sz / bucketSize;
        tableSize = nBuckets * bucketSize;
    }

    size_t increaseBucketSize(size_t sz, size_t hashCollisions) {
        if (bucketSize != sz) return 0;

        do {
            bucketSize = bucketSize < LINEAR_INCREMENT_THRESHOLD
                ? bucketSize * 2
                : bucketSize + LINEAR_INCREMENT_THRESHOLD;
        } while (bucketSize < hashCollisions);

        size_t newNBuckets = nBuckets;
        while (newNBuckets * bucketSize > tableSize) {
            nBuckets = newNBuckets;
            newNBuckets = nBuckets / 2 < LINEAR_INCREMENT_THRESHOLD
                ? nBuckets / 2
                : nBuckets - LINEAR_INCREMENT_THRESHOLD;
        }

        size_t oldSize = tableSize;

        tableSize = nBuckets * bucketSize;

        return oldSize;
    }

    size_t increaseNumberOfBuckets(size_t sz) {
        if (nBuckets != sz) return 0;

        nBuckets = nBuckets < LINEAR_INCREMENT_THRESHOLD
            ? nBuckets * 2
            : nBuckets + LINEAR_INCREMENT_THRESHOLD;

        size_t oldSize = tableSize;

        tableSize = nBuckets * bucketSize;

        return oldSize;
    }

    bool needToExpandBucket(size_t hashCollisionsInRange) const {
        return hashCollisionsInRange > bucketSize;
    }

    HashRange hashRange(size_t h) const {
        size_t i = h % nBuckets * bucketSize;
        size_t j = (i + BUCKET_SEARCH * bucketSize) % tableSize;
        return { i, j, tableSize };
    }

    void enter() const {
    }

    void exit() const {
    }

    void lock() {
    }

    void unlock() {
    }
};

struct ConcurrentHashTableController
:   HashTableController
{
    std::mutex _mtx;
    std::atomic_bool _hold = false;
    std::atomic_size_t _resident = 0;

    ConcurrentHashTableController() = default;

    ConcurrentHashTableController(const ConcurrentHashTableController &rhs)
    :   HashTableController(rhs)
    { }

    ConcurrentHashTableController(ConcurrentHashTableController &&rhs)
    :   HashTableController(std::move(rhs))
    { }

    ConcurrentHashTableController & operator=(const ConcurrentHashTableController &rhs) {
        HashTableController::operator=(rhs);
        return *this;
    }

    ConcurrentHashTableController & operator=(ConcurrentHashTableController &&rhs) {
        HashTableController::operator=(std::move(rhs));
        return *this;
    }

    void enter() const {
        while (_hold.load(std::memory_order_relaxed)) sched_yield();
        ++const_cast<ConcurrentHashTableController *>(this)->_resident;
    }

    void exit() const {
        --const_cast<ConcurrentHashTableController *>(this)->_resident;
    }

    void lock() {
        --_resident;
        _mtx.lock();
        _hold.store(true, std::memory_order_release);
        while (_resident.load(std::memory_order_relaxed) > 0) sched_yield();
    }

    void unlock() {
        ++_resident;
        _hold.store(false, std::memory_order_release);
        _mtx.unlock();
    }
};

template <
    typename Key,
    typename node,
    typename KeyHash,
    typename NodeKeyEqual,
    typename Controller,
    typename size_type
>
class HashTable {
protected:

    using storage_node = typename node::storage_type;

    static constexpr size_t __INITIAL_TABLE_SIZE = 128;
    static constexpr size_t __NPOS = (size_t) -1;

    KeyHash _hash;
    NodeKeyEqual _eq;

    Controller _controller;
    node *_table;       // table
    size_type _size;    // number of elements

    template <typename X>
    class HashTableIterator
    :   public BidirectionalIterator<HashTableIterator<X>, X>
    {
    protected:

        size_t _i = 0;
        const HashTable *_ht = nullptr;
        node *_table = nullptr;
        size_t _size = 0;

    public:

        using difference_type = typename BidirectionalIterator<HashTableIterator<X>, X>::difference_type;
        using reference = typename BidirectionalIterator<HashTableIterator<X>, X>::reference;
        using pointer = typename BidirectionalIterator<HashTableIterator<X>, X>::pointer;

        HashTableIterator(size_t index, const HashTable *ht)
        :   _i(index),
            _ht(ht)
        {
            _ht->_controller.enter();
            _table = _ht->_table;
            _size = _ht->_controller.tableSize;

            if (ht->_size == 0) {
                _i = _size;
            }
            else {
                while (_i < _size && ! _table[_i].occupied()) ++_i;
            }
        }

        HashTableIterator(const HashTableIterator &rhs)
        :   _i(rhs._i),
            _ht(rhs._ht)
        {
            _ht->_controller.enter();
            _table = _ht->_table;
            _size = _ht->_controller.tableSize;
        }

        HashTableIterator(HashTableIterator &&rhs)
        :   _i(rhs._i),
            _ht(rhs._ht),
            _table(rhs._table),
            _size(rhs._size)
        {
            rhs._ht = nullptr;
        }

        ~HashTableIterator() {
            if (_ht != nullptr) _ht->_controller.exit();
        }

        HashTableIterator & operator=(const HashTableIterator &rhs) {
            if (this != &rhs) {
                if (_ht != nullptr) _ht->_controller.exit();

                _i = rhs._i;
                _ht = rhs._ht;

                _ht->_controller.enter();
                _table = _ht->_table;
                _size = _ht->_controller.tableSize;
            }
            return *this;
        }

        HashTableIterator & operator=(HashTableIterator &&rhs) {
            if (this != &rhs) {
                if (_ht != nullptr) _ht->_controller.exit();

                _i = rhs._i;
                _ht = rhs._ht;
                _table = rhs._table;
                _size = rhs._size;

                rhs._ht = nullptr;
            }
            return *this;
        }

        bool operator==(const HashTableIterator &rhs) const { return _i == rhs._i; }
        bool operator!=(const HashTableIterator &rhs) const { return _i != rhs._i; }
        bool operator< (const HashTableIterator &rhs) const { return _i <  rhs._i; }
        bool operator<=(const HashTableIterator &rhs) const { return _i <= rhs._i; }
        bool operator> (const HashTableIterator &rhs) const { return _i >  rhs._i; }
        bool operator>=(const HashTableIterator &rhs) const { return _i >= rhs._i; }

        reference operator*() {
            return _table[_i].storage.n;
        }

        pointer operator->() {
            return &_table[_i].storage.n;
        }

        HashTableIterator & operator++() {
            do {
                ++_i;
            } while (_i < _size && ! _table[_i].occupied());
            return *this;
        }

        HashTableIterator operator++(int) {
            auto i = _i;
            operator++();
            return HashTableIterator(i, _ht);
        }

        HashTableIterator & operator--() {
            do {
                --_i;
            } while (_i > 0 && ! _table[_i].occupied());
            return *this;
        }

        HashTableIterator operator--(int) {
            auto i = _i;
            operator--();
            return HashTableIterator(i, _ht);
        }
    };

    void _resize(size_t hashCollisions) {
        // more collisions than current bucket size
        if (
            _controller.needToExpandBucket(hashCollisions)
            || _controller.tableSize >= 2 * _size
        ) {
            // snapshot of bucketSize before lock
            size_t s = _controller.bucketSize;

            // bucket size may change while we acquire the lock
            _controller.lock();

            // if we are actually responsible for an increase, rehash
            size_t oldTableSize = _controller.increaseBucketSize(s, hashCollisions);
            if (oldTableSize != 0) {
                if (_controller.tableSize == _size) {
                    _controller.increaseNumberOfBuckets(_controller.nBuckets);
                }
                _rehash(oldTableSize);
            }

            _controller.unlock();
        }
        else {
            // snapshot of the number of buckets before lock
            size_t s = _controller.nBuckets;

            // number of buckets may change while we acquire the lock
            _controller.lock();

            // if we are actually responsible for an increase, rehash
            size_t oldTableSize = _controller.increaseNumberOfBuckets(s);
            if (oldTableSize != 0){
                _rehash(oldTableSize);
            }

            _controller.unlock();
        }
    }

    HashRange _findRange(size_t h) const {
        return _controller.hashRange(h);
    }

    template <typename K>
    size_t _findNext(HashRange &range, size_t h, const K &k) const {
        do {
            if (
                _table[range.pos].occupied()
                && _table[range.pos].h == h
                && _eq(_table[range.pos], k)
            ) {
                size_t retval = range.pos;
                range.next();
                return retval;
            }
            range.next();
        } while (range.nonEmpty());

        return __NPOS;
    }

    template <typename K>
    size_t _findIndex(size_t h, const K &k) const {
        // range of indices where our key should exist
        HashRange range = _controller.hashRange(h);

        do {
            if (
                _table[range.pos].occupied()
                && _table[range.pos].h == h
                && _eq(_table[range.pos], k)
            ) return range.pos;
            range.next();
        } while (range.nonEmpty());

        return __NPOS;
    }

    size_t _getFreeIndex(size_t h) {
        HashRange range = _controller.hashRange(h);
        size_t collisions = 1;          // count hash collisions

        do {
            if (_table[range.pos].occupyIfFree()) return range.pos;
            if (_table[range.pos].h == h) ++collisions;
            range.next();
        } while (range.nonEmpty());

        _resize(collisions);
        return _getFreeIndex(h);
    }

    template <typename K>
    size_t _findOrGetFreeIndex(size_t h, const K &k) {
        // range of indices where our key should exist
        HashRange range = _controller.hashRange(h);
        size_t collisions = 1;          // count hash collisions

        do {
            if (_table[range.pos].occupyIfFree()) return range.pos;
            if (_table[range.pos].h == h) {
                if (_eq(_table[range.pos], k)) return range.pos;
                ++collisions;
            }
            range.next();
        } while (range.nonEmpty());

        _resize(collisions);
        return _getFreeIndex(h);
    }

    size_t _getFreeIndex_noResize(size_t h) {
        HashRange range = _controller.hashRange(h);

        do {
            if (_table[range.pos].occupyIfFree()) return range.pos;
            range.next();
        } while (range.nonEmpty());

        throw 0;        // shouldn't get here
    }

    void _rehash(size_t oldTableSize) {
        node *old = _table;
        _table = new node[_controller.tableSize];

        for (size_t i = 0; i < oldTableSize; ++i) {
            if (old[i].occupied()) {
                size_t j = _getFreeIndex_noResize(old[i].h);
                _table[j].set(old[i].h, std::move(old[i].storage.n));
                old[i].release();
            }
        }

        delete[] old;
    }

    void _move(HashTable &rhs) {
        _controller = rhs._controller;
        _table = rhs._table;
        _size = static_cast<size_t>(rhs._size);
    }

    void _copy(const HashTable &rhs) {
        rhs._controller.enter();
        _controller = rhs._controller;
        _table = new node[_controller.tableSize];
        _size = rhs._size;
        for (size_t i = 0; i < _controller.tableSize; ++i) {
            if (rhs._table[i].occupied()) {
                size_t j = _getFreeIndex_noResize(rhs._table[i].h);
                _table[j].set(rhs._table[i].h, rhs._table[i].storage.n);
            }
        }
        rhs._controller.exit();
    }

    void _freeNodes() {
        for (size_t i = 0, sz = _size; i < _controller.tableSize && sz > 0; ++i) {
            if (_table[i].occupied()) {
                _table[i].release();
                --sz;
            }
        }
    }

    void _dispose() {
        if (_table != nullptr) delete[] _table;
    }

    void _invalidate() {
        _controller = Controller();
        _table = nullptr;
        _size = 0;
    }

    template <
        typename X = storage_node,
        std::enable_if_t<
            SupportsTrivialSerialization<X> && ! SupportsCustomSerialization<X>
        , int> = 0
    >
    void _serialize(OutputStreamSerializer &serializer, SerializationLevel level) const {
        _controller.enter();

        size_t sz = static_cast<size_t>(_size);

        serializer << _controller << sz;

        if (level == SerializationLevel::PLAIN) {
            serializer.put(_table, sizeof(node) * _controller.tableSize);
        }
        else {
            for (size_t i = 0; i < _controller.tableSize && sz > 0; ++i) {
                if (_table[i].occupied()) {
                    serializer << _table[i].h;
                    serializer << _table[i].storage.n;
                    --sz;
                }
            }
        }

        _controller.exit();
    }

    template <
        typename X = storage_node,
        std::enable_if_t<
            SupportsTrivialSerialization<X> && ! SupportsCustomSerialization<X>
            && std::is_constructible_v<X>
        , int> = 0
    >
    void _deserialize(InputStreamSerializer &serializer, SerializationLevel level) {
        _freeNodes();
        _dispose();

        size_t sz;

        serializer >> _controller >> sz;

        _table = new node[_controller.tableSize];
        _size = sz;

        if (level == SerializationLevel::PLAIN) {
            serializer.get(_table, sizeof(node) * _controller.tableSize);
        }
        else {
            for (size_t i = 0; i < sz ; ++i) {
                size_t h;
                storage_node n;
                serializer >> h;
                serializer >> n;
                size_t j = _getFreeIndex_noResize(h);
                _table[j].set(h, std::move(n));
            }
        }
    }

    template <
        typename X = storage_node,
        std::enable_if_t<
            SupportsCustomSerialization<X>
        , int> = 0
    >
    void _serialize(OutputStreamSerializer &serializer, SerializationLevel level) const {
        _controller.enter();

        size_t sz = static_cast<size_t>(_size);

        serializer << _controller << sz;

        for (size_t i = 0; i < _controller.tableSize && sz > 0; ++i) {
            if (_table[i].occupied()) {
                serializer << _table[i].h;
                _table[i].storage.n.writeObject(serializer, level);
                --sz;
            }
        }

        _controller.exit();
    }

    template <
        typename X = storage_node,
        std::enable_if_t<
            SupportsCustomSerialization<X> && std::is_constructible_v<X>
        , int> = 0
    >
    void _deserialize(InputStreamSerializer &serializer, SerializationLevel level) {
        _freeNodes();
        _dispose();

        size_t sz;

        serializer >> _controller >> sz;

        _table = new node[_controller.tableSize];
        _size = sz;

        for (size_t i = 0; i < sz ; ++i) {
            size_t h;
            storage_node n;
            serializer >> h;
            n.readObject(serializer, level);
            size_t j = _getFreeIndex_noResize(h);
            _table[j].set(h, std::move(n));
        }
    }

    template <
        typename X = storage_node,
        std::enable_if_t<! SupportsSerialization<X>, int> = 0
    >
    void _serialize(OutputStreamSerializer &serializer, SerializationLevel level) const {
        throw DynamicMessageError(
            "Type '", typeid(storage_node).name(), "' cannot be serialized."
        );
    }

    template <
        typename X = storage_node,
        std::enable_if_t<! SupportsSerialization<X> || ! std::is_constructible_v<X>, int> = 0
    >
    void _deserialize(InputStreamSerializer &serializer, SerializationLevel level) {
        throw DynamicMessageError(
            "Type '", typeid(storage_node).name(), "' cannot be deserialized."
        );
    }

public:

    HashTable(size_t initialSize = __INITIAL_TABLE_SIZE) {
        _controller.init(initialSize);
        _table = new node[_controller.tableSize];
        _size = 0;
    }

    HashTable(const HashTable &rhs) {
        _copy(rhs);
    }

    HashTable(HashTable &&rhs) {
        _move(rhs);
        rhs._invalidate();
    }

    ~HashTable() {
        _controller.enter();
        _controller.lock();
        _freeNodes();
        _dispose();
    }

    HashTable & operator=(const HashTable &rhs) {
        if (this != &rhs) {
            _controller.enter();
            _controller.lock();
            _freeNodes();
            _dispose();
            _copy(rhs);
            _controller.unlock();
            _controller.exit();
        }
        return *this;
    }

    HashTable & operator=(HashTable &&rhs) {
        if (this != &rhs) {
            _controller.enter();
            _controller.lock();
            _freeNodes();
            _dispose();
            _move(rhs);
            rhs._invalidate();
            _controller.unlock();
            _controller.exit();
        }
        return *this;
    }

    size_t size() const {
        return _size;
    }

    bool empty() const {
        return _size == 0;
    }

    auto cbegin() const {
        return HashTableIterator<const storage_node>(0, this);
    }
    auto begin() {
        return HashTableIterator<storage_node>(0, this);
    }
    auto begin() const {
        return cbegin();
    }

    auto cend() const {
        return HashTableIterator<const storage_node>(_controller.tableSize, this);
    }
    auto end() {
        return HashTableIterator<storage_node>(_controller.tableSize, this);
    }
    auto end() const {
        return cend();
    }

    void clear() {
        _controller.enter();
        _controller.lock();
        _freeNodes();
        _dispose();
        _controller.init(__INITIAL_TABLE_SIZE);
        _table = new node[_controller.tableSize];
        _size = 0;
        _controller.unlock();
        _controller.exit();
    }
};

}   // __HashTable

}   // namespace spl
