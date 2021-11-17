#include <serialization.h>
#include <factory.h>
#include <dtest.h>

using namespace spl;

struct StreamSerializable
:   Serializable,
    WithDefaultFactory<StreamSerializable>
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
