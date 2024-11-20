/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <serialization.h>
#include "test_serializers.cpp"
#include <algorithm>

module("stream-serializer")
.dependsOn({
    "exception",
    "factory"
});

module("random-access-serializer")
.dependsOn({
    "stream-serializer"
});

using namespace spl;

#define TEST_SIZE (1024)

unit("stream-serializer", "primitive-types")
.body([] {
    int x = 1;
    long y = 2;
    short z = 3;
    struct A {
        int a;
        int b;
    };

    A w;
    w.a = 10;
    w.b = 11;

    MemoryOutputStreamSerializer out;
    out << x << y << z << w;
    out.flush();

    int a;
    long b;
    short c;
    A d;

    auto &in = *out.toInput();
    in >> a >> b >> c >> d;
    delete &in;

    assert(a == x);
    assert(b == y);
    assert(c == z);
    assert(d.a == w.a);
    assert(d.b == w.b);
});

unit("stream-serializer", "serializable-type")
.body([] {

    struct StreamSerializable
    :   Serializable,
        WithFactory<StreamSerializable>
    {
        int data = 5;

        void writeObject(OutputStreamSerializer &serializer) const override {
            serializer << data;
        }

        void writeObject(OutputRandomAccessSerializer &serializer) const override {
            assert(false);
        }

        void readObject(InputStreamSerializer &serializer) override {
            int x;
            serializer >> x;
            assert(x == 5);
        }

        void readObject(InputRandomAccessSerializer &serializer) override {
            assert(false);
        }
    };

    StreamSerializable a, b;

    MemoryOutputStreamSerializer out;
    out << a;
    out.flush();

    auto &in = *out.toInput();
    in >> b;
    delete &in;
});

unit("stream-serializer", "serializable-type-ptr")
.body([] {

    struct StreamSerializable
    :   Serializable,
        WithFactory<StreamSerializable>
    {
        int data = 5;

        void writeObject(OutputStreamSerializer &serializer) const override {
            serializer << data;
        }

        void writeObject(OutputRandomAccessSerializer &serializer) const override {
            assert(false);
        }

        void readObject(InputStreamSerializer &serializer) override {
            int x;
            serializer >> x;
            assert(x == 5);
        }

        void readObject(InputRandomAccessSerializer &serializer) override {
            assert(false);
        }
    };

    StreamSerializable *a = new StreamSerializable(), *b = new StreamSerializable();

    MemoryOutputStreamSerializer out;
    out << a;
    out.flush();

    auto &in = *out.toInput();
    in >> b;
    delete &in;

    delete a;
    delete b;
});

unit("stream-serializer", "serializable-type-ptr-creation")
.body([] {

    struct StreamSerializable
    :   Serializable,
        WithFactory<StreamSerializable>
    {
        int data = 5;

        void writeObject(OutputStreamSerializer &serializer) const override {
            serializer << data;
        }

        void writeObject(OutputRandomAccessSerializer &serializer) const override {
            assert(false);
        }

        void readObject(InputStreamSerializer &serializer) override {
            int x;
            serializer >> x;
            assert(x == 5);
        }

        void readObject(InputRandomAccessSerializer &serializer) override {
            assert(false);
        }
    };

    StreamSerializable *a = new StreamSerializable(), *b = nullptr;

    MemoryOutputStreamSerializer out;
    out << a;
    out.flush();

    auto &in = *out.toInput();
    in >> b;
    delete &in;

    assert(b != nullptr);
    assert(a->objectCode() == b->objectCode());

    delete a;
    delete b;
});

unit("stream-serializer", "serializable-type-nullptr")
.body([] {

    struct StreamSerializable
    :   Serializable,
        WithFactory<StreamSerializable>
    {
        int data = 5;

        void writeObject(OutputStreamSerializer &serializer) const override {
            serializer << data;
        }

        void writeObject(OutputRandomAccessSerializer &serializer) const override {
            assert(false);
        }

        void readObject(InputStreamSerializer &serializer) override {
            int x;
            serializer >> x;
            assert(x == 5);
        }

        void readObject(InputRandomAccessSerializer &serializer) override {
            assert(false);
        }
    };

    StreamSerializable *a = nullptr, *b = new StreamSerializable();

    MemoryOutputStreamSerializer out;
    out << a;
    out.flush();

    auto &in = *out.toInput();
    in >> b;
    delete &in;

    assert(b == nullptr);
});

unit("stream-serializer", "large-serialization")
.body([] {

    int *a = new int[TEST_SIZE];
    MemoryOutputStreamSerializer out;
    for (auto i = 0; i < TEST_SIZE; ++i) {
        a[i] = dtest_random() * TEST_SIZE;
        out << a[i];
    }
    out.flush();

    int *b = new int[TEST_SIZE];
    auto &in = *out.toInput();
    for (auto i = 0; i < TEST_SIZE; ++i) {
        in >> b[i];
    }
    delete &in;

    assert(memcmp(a, b, TEST_SIZE * sizeof(int)) == 0);

    delete[] a;
    delete[] b;
});

unit("stream-serializer", "bulk-serialization")
.body([] {

    int *a = new int[TEST_SIZE];
    MemoryOutputStreamSerializer out;
    for (auto i = 0; i < TEST_SIZE; ++i) {
        a[i] = dtest_random() * TEST_SIZE;
    }
    out.put(a, TEST_SIZE * sizeof(int));
    out.flush();

    int *b = new int[TEST_SIZE];
    auto &in = *out.toInput();
    in.get(b, TEST_SIZE * sizeof(int));
    delete &in;

    assert(memcmp(a, b, TEST_SIZE * sizeof(int)) == 0);

    delete[] a;
    delete[] b;
});

unit("stream-serializer", "lock")
.body([] {
    MemoryOutputStreamSerializer out;
    out << 1;
    out.lock();
    out << 2;
    out.flush();

    auto &in = *out.toInput();
    int x;
    in >> x;
    assert(x == 1);

    try {
        in >> x;
        delete &in;
        fail("got data beyond lock position");
    }
    catch (const OutOfRangeError &) { }

    delete &in;
});

unit("random-access-serializer", "primitive-types")
.body([] {
    int x = 1;
    long y = 2;
    short z = 3;
    struct A {
        int a;
        int b;
    };

    A w;
    w.a = 10;
    w.b = 11;

    MemoryOutputRandomAccessSerializer out;
    out << x << y << z << w;
    out.flush();

    int a;
    long b;
    short c;
    A d;

    auto &in = *out.toInput();
    in >> a >> b >> c >> d;
    delete &in;

    assert(a == x);
    assert(b == y);
    assert(c == z);
    assert(d.a == w.a);
    assert(d.b == w.b);
});

unit("random-access-serializer", "serializable-type")
.body([] {

    struct RandomAccessSerializable
    :   Serializable,
        WithFactory<RandomAccessSerializable>
    {
        int data = 5;

        void writeObject(OutputStreamSerializer &serializer) const override {
            assert(false);
        }

        virtual void writeObject(OutputRandomAccessSerializer &serializer) const {
            serializer << data;
        }

        virtual void readObject(InputStreamSerializer &serializer) {
            assert(false);
        }

        virtual void readObject(InputRandomAccessSerializer &serializer) {
            int x;
            serializer >> x;
            assert(x == 5);
        }
    };

    RandomAccessSerializable a, b;

    MemoryOutputRandomAccessSerializer out;
    out << a;
    out.flush();

    auto &in = *out.toInput();
    in >> b;
    delete &in;
});

unit("random-access-serializer", "large-serialization")
.body([] {

    int *idx_write = new int[TEST_SIZE];
    int *idx_read = new int[TEST_SIZE];
    for (auto i = 0; i < TEST_SIZE; ++i) {
        idx_write[i] = i;
    }
    memcpy(idx_read, idx_write, TEST_SIZE * sizeof(int));
    std::random_shuffle(idx_write, idx_write + TEST_SIZE);
    std::random_shuffle(idx_read, idx_read + TEST_SIZE);

    int *a = new int[TEST_SIZE];
    MemoryOutputRandomAccessSerializer out;
    for (auto i = 0; i < TEST_SIZE; ++i) {
        a[i] = dtest_random() * TEST_SIZE;
    }
    for (auto i = 0; i < TEST_SIZE; ++i) {
        out.seekTo(idx_write[i] * sizeof(int));
        out << a[idx_write[i]];
    }
    out.flush();

    int *b = new int[TEST_SIZE];
    auto &in = *out.toInput();
    for (auto i = 0; i < TEST_SIZE; ++i) {
        in.seekTo(idx_read[i] * sizeof(int));
        in >> b[idx_read[i]];
    }
    delete &in;

    assert(memcmp(a, b, TEST_SIZE * sizeof(int)) == 0);

    delete[] a;
    delete[] b;
    delete[] idx_write;
    delete[] idx_read;
});

unit("random-access-serializer", "tell")
.inProcess()
.body([] {

    MemoryOutputRandomAccessSerializer out;

    assert(out.tell() == 0);
    for (auto i = 0; i < 3; ++i) {
        out << static_cast<int>(dtest_random() * 3);
    }
    assert(out.tell() == sizeof(int) * 3);

    out.flush();
    assert(out.tell() == sizeof(int) * 3);

    auto &in = *out.toInput();
    assert(in.tell() == 0);
    for (auto i = 0; i < 3; ++i) {
        int x;
        in >> x;
    }
    assert(in.tell() == sizeof(int) * 3);
    delete &in;
});
