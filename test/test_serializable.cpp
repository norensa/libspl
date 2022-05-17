/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <serialization.h>
#include <factory.h>
#include <dtest.h>

using namespace spl;

struct StreamSerializable
:   Serializable,
    WithFactory<StreamSerializable>
{
    int data = 0;

    void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
        const_cast<StreamSerializable *>(this)->data = -1;
        serializer << data;
    }

    void writeObject(OutputRandomAccessSerializer &serializer, SerializationLevel level) const override {
        assert(false);
    }

    void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
        serializer >> data;
        assert(data == -1);
        data = 1;
    }

    void readObject(InputRandomAccessSerializer &serializer, SerializationLevel level) override {
        assert(false);
    }

    bool serialized() const {
        return data == -1;
    }

    bool deserialized() const {
        return data == 1;
    }
};

struct StreamSerializable_NotConstructible {
    StreamSerializable_NotConstructible(int) { }
};

struct StreamSerializable_NotCopyAssignable {
    StreamSerializable_NotCopyAssignable & operator=(const StreamSerializable_NotCopyAssignable &) = delete;
};

struct ComparableStreamSerializable
:   StreamSerializable,
    WithFactory<ComparableStreamSerializable>
{
    int x = 0;

    ComparableStreamSerializable() = default;

    ComparableStreamSerializable(int x)
    :   x(x)
    { }

    void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
        StreamSerializable::writeObject(serializer, level);
        serializer << x;
    }

    void writeObject(OutputRandomAccessSerializer &serializer, SerializationLevel level) const override {
        assert(false);
    }

    void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
        StreamSerializable::readObject(serializer, level);
        serializer >> x;
    }

    void readObject(InputRandomAccessSerializer &serializer, SerializationLevel level) override {
        assert(false);
    }

    bool operator<(const ComparableStreamSerializable &rhs) const {
        return x < rhs.x;
    }
};

struct RandomAccessSerializable
:   Serializable,
    WithFactory<RandomAccessSerializable>
{
    int data = 0;

    void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
        assert(false);
    }

    void writeObject(OutputRandomAccessSerializer &serializer, SerializationLevel level) const override {
        const_cast<RandomAccessSerializable *>(this)->data = -1;
        serializer << data;
    }

    void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
        assert(false);
    }

    void readObject(InputRandomAccessSerializer &serializer, SerializationLevel level) override {
        serializer >> data;
        assert(data == -1);
        data = 1;
    }

    bool serialized() const {
        return data == -1;
    }

    bool deserialized() const {
        return data == 1;
    }
};
