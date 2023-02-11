/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <hash.h>
#include <traits.h>
#include <serialization.h>

using namespace spl;

struct HashableObj : Hashable {
    long v;
    void *buf;

    HashableObj()
    :   v(-1),
        buf(nullptr)
    { }

    HashableObj(long v)
    :   v(v),
        buf(malloc(1))
    { }

    HashableObj(const HashableObj &r)
    :   v(r.v),
        buf(malloc(1))
    { }

    HashableObj(HashableObj &&r)
    :   v(r.v),
        buf(r.buf)
    {
        r.buf = nullptr;
    }

    ~HashableObj() {
        if (buf != nullptr) {
            free(buf);
            buf = nullptr;
        }
    }

    HashableObj & operator=(const HashableObj &r) {
        if (this != &r) {
            if (buf != nullptr) free(buf);
            v = r.v;
            buf = malloc(1);
        }
        return *this;
    }

    HashableObj & operator=(HashableObj &&r) {
        if (this != &r) {
            if (buf != nullptr) free(buf);
            v = r.v;
            buf = r.buf;
            r.buf = nullptr;
        }
        return *this;
    }

    size_t hash() const {
        return (size_t) v;
    }

    bool operator==(const HashableObj &rhs) const {
        return v == rhs.v;
    }
};

struct HashableSerializableObj : HashableObj, Serializable {
    bool serialized = false;
    bool deserialized = false;

    HashableSerializableObj() = default;

    HashableSerializableObj(long v)
    :   HashableObj(v)
    { }

    void writeObject(OutputStreamSerializer &serializer) const override {
        serializer << v;
        const_cast<HashableSerializableObj *>(this)->serialized = true;
    }

    void readObject(InputStreamSerializer &serializer) {
        serializer >> v;
        if (buf != nullptr) free(buf);
        buf = malloc(1);
        deserialized = true;
    }
};
