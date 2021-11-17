#include <dtest.h>
#include <serialization.h>
#include "test_serializers.cpp"

module("stream-serializer")
.dependsOn({
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

    auto in = out.toInput();
    in >> a >> b >> c >> d;

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
        WithDefaultFactory<StreamSerializable>
    {
        int data = 5;

        void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
            serializer << data;
        }

        void writeObject(OutputRandomAccessSerializer &serializer, SerializationLevel level) const override {
            assert(false);
        }

        void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
            int x;
            serializer >> x;
            assert(x == 5);
        }

        void readObject(InputRandomAccessSerializer &serializer, SerializationLevel level) override {
            assert(false);
        }
    };

    StreamSerializable a, b;

    MemoryOutputStreamSerializer out;
    out << a;
    out.flush();

    auto in = out.toInput();
    in >> b;
});

unit("stream-serializer", "serializable-type-ptr")
.body([] {

    struct StreamSerializable
    :   Serializable,
        WithDefaultFactory<StreamSerializable>
    {
        int data = 5;

        void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
            serializer << data;
        }

        void writeObject(OutputRandomAccessSerializer &serializer, SerializationLevel level) const override {
            assert(false);
        }

        void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
            int x;
            serializer >> x;
            assert(x == 5);
        }

        void readObject(InputRandomAccessSerializer &serializer, SerializationLevel level) override {
            assert(false);
        }
    };

    StreamSerializable *a = new StreamSerializable(), *b = new StreamSerializable();

    MemoryOutputStreamSerializer out;
    out << a;
    out.flush();

    auto in = out.toInput();
    in >> b;

    delete a;
    delete b;
});

unit("stream-serializer", "serializable-type-ptr-creation")
.body([] {

    struct StreamSerializable
    :   Serializable,
        WithDefaultFactory<StreamSerializable>
    {
        int data = 5;

        void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
            serializer << data;
        }

        void writeObject(OutputRandomAccessSerializer &serializer, SerializationLevel level) const override {
            assert(false);
        }

        void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
            int x;
            serializer >> x;
            assert(x == 5);
        }

        void readObject(InputRandomAccessSerializer &serializer, SerializationLevel level) override {
            assert(false);
        }
    };

    StreamSerializable *a = new StreamSerializable(), *b = nullptr;

    MemoryOutputStreamSerializer out;
    out << a;
    out.flush();

    auto in = out.toInput();
    in >> b;

    assert(b != nullptr);
    assert(a->objectCode() == b->objectCode());

    delete a;
    delete b;
});

unit("stream-serializer", "serializable-type-nullptr")
.body([] {

    struct StreamSerializable
    :   Serializable,
        WithDefaultFactory<StreamSerializable>
    {
        int data = 5;

        void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
            serializer << data;
        }

        void writeObject(OutputRandomAccessSerializer &serializer, SerializationLevel level) const override {
            assert(false);
        }

        void readObject(InputStreamSerializer &serializer, SerializationLevel level) override {
            int x;
            serializer >> x;
            assert(x == 5);
        }

        void readObject(InputRandomAccessSerializer &serializer, SerializationLevel level) override {
            assert(false);
        }
    };

    StreamSerializable *a = nullptr, *b = new StreamSerializable();

    MemoryOutputStreamSerializer out;
    out << a;
    out.flush();

    auto in = out.toInput();
    in >> b;

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
    auto in = out.toInput();
    for (auto i = 0; i < TEST_SIZE; ++i) {
        in >> b[i];
    }

    assert(memcmp(a, b, TEST_SIZE * sizeof(int)) == 0);

    delete a;
    delete b;
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
    auto in = out.toInput();
    in.get(b, TEST_SIZE * sizeof(int));

    assert(memcmp(a, b, TEST_SIZE * sizeof(int)) == 0);

    delete a;
    delete b;
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

    auto in = out.toInput();
    in >> a >> b >> c >> d;

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
        WithDefaultFactory<RandomAccessSerializable>
    {
        int data = 5;

        void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const override {
            assert(false);
        }

        virtual void writeObject(OutputRandomAccessSerializer &serializer, SerializationLevel level) const {
            serializer << data;
        }

        virtual void readObject(InputStreamSerializer &serializer, SerializationLevel level) {
            assert(false);
        }

        virtual void readObject(InputRandomAccessSerializer &serializer, SerializationLevel level) {
            int x;
            serializer >> x;
            assert(x == 5);
        }
    };

    RandomAccessSerializable a, b;

    MemoryOutputRandomAccessSerializer out;
    out << a;
    out.flush();

    auto in = out.toInput();
    in >> b;
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
    auto in = out.toInput();
    for (auto i = 0; i < TEST_SIZE; ++i) {
        in.seekTo(idx_read[i] * sizeof(int));
        in >> b[idx_read[i]];
    }

    assert(memcmp(a, b, TEST_SIZE * sizeof(int)) == 0);

    delete a;
    delete b;
    delete idx_write;
    delete idx_read;
});
