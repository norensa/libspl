/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <cstdlib>
#include <stdint.h>
#include <factory.h>
#include <exception.h>

namespace spl {

class OutputStreamSerializer;
class OutputRandomAccessSerializer;
class InputStreamSerializer;
class InputRandomAccessSerializer;

/**
 * @brief An enumeration to indicate serialization preference.
*/
enum class SerializationLevel : uint8_t {
    /**
     * @brief PLAIN serialization level indicates that no compression or
     * encoding may be used. This level of serialization allows fast and
     * efficient transfer of data when bandwidth utilization is not a
     * concern.
     */
    PLAIN,

    /**
     * @brief COMPACTED serialization level indicates that some simple
     * encoding or compression technique may be used; e.g. run-length
     * encoding or bit-packing. This allows efficient bandwidth use at
     * the expense of some trivial compute work.
     */
    COMPACTED,

    /**
     * @brief COMPRESSED serialization level indicates that a more compute
     * intensive compression may be used because bandwidth is limited.
     * Lightweight compression algorithms may be used.
     */
    COMPRESSED,

    /**
     * @brief COMPRESSED_2 serialization level indicates that a best-effort
     * compression is needed due to extremely limited bandwidth. Any amount
     * of compute work may be used to produce the smallest possible data size.
     */
    COMPRESSED_2,
};

/**
 * @brief Base class for all serializable objects.
*/
class Serializable {
private:

    size_t _objectCode = 0;

public:

    Serializable()
    :   _objectCode(0)
    { }

    virtual ~Serializable() = default;

    /**
     * @return A non-zero hash code unique to this class type.
     */
    size_t objectCode() const {
        if (_objectCode == 0) {
            size_t h = typeid(*this).hash_code();
            const_cast<Serializable *>(this)->_objectCode = h == 0 ? 1 : h;
        }
        return _objectCode;
    }

    /**
     * @brief Writes this object to a stream serializer.
     * 
     * @param[in] serializer Reference to a stream serializer instance.
     * @param[in] level The requested serialization level.
     */
    virtual void writeObject(OutputStreamSerializer &serializer, SerializationLevel level) const = 0;

    /**
     * @brief Writes this object to a random access serializer. The default
     * implementation delegates this call to the implemented stream
     * serialization.
     * 
     * @param[in] serializer Reference to a random access serializer instance.
     * @param[in] level The requested serialization level.
     */
    virtual void writeObject(OutputRandomAccessSerializer &serializer, SerializationLevel level) const {
        writeObject((OutputStreamSerializer &) serializer, level);
    }

    /**
     * @brief Reads this object from a stream serializer.
     * 
     * @param[in] serializer Reference to a stream serializer instance.
     * @param[in] level The requested serialization level.
     */
    virtual void readObject(InputStreamSerializer &serializer, SerializationLevel level) = 0;

    /**
     * @brief Reads this object from a random access serializer. The default
     * implementation delegates this call to the implemented stream
     * serialization.
     * 
     * @param[in] serializer Reference to a random access serializer instance.
     * @param[in] level The requested serialization level.
     */
    virtual void readObject(InputRandomAccessSerializer &serializer, SerializationLevel level) {
        readObject((InputStreamSerializer &) serializer, level);
    }
};

/**
 * @brief A type trait to check if T is trivially serializable. 
 * 
 * @tparam T The type to check.
 */
template <typename T>
struct SupportsTrivialSerialization_t {
    static constexpr bool value = std::is_copy_assignable_v<T>;
};

/**
 * @brief A type trait to check if T implements custom serialization. 
 * 
 * @tparam T The type to check.
 */
template <typename T>
struct SupportsCustomSerialization_t {
    static constexpr bool value = std::is_base_of_v<Serializable, std::remove_pointer_t<T>>;
};

/**
 * @brief Evaluates to true if T is trivially serializable. 
 * 
 * @tparam T The type to check.
 */
template <typename T>
inline constexpr bool SupportsTrivialSerialization = SupportsTrivialSerialization_t<T>::value;

/**
 * @brief Evaluates to true if T implements custom serialization. 
 * 
 * @tparam T The type to check.
 */
template <typename T>
inline constexpr bool SupportsCustomSerialization = SupportsCustomSerialization_t<T>::value;

/**
 * @brief Evaluates to true if T is serializable. 
 * 
 * @tparam T The type to check.
 */
template <typename T>
inline constexpr bool SupportsSerialization = (
    SupportsTrivialSerialization<T>
    || SupportsCustomSerialization<T>
);

/**
 * @brief A serializer used for writing objects to some underlying byte stream.
*/
class OutputStreamSerializer {

protected:

    static constexpr size_t _DEFAULT_BUFFER_SIZE = 1024;

    /**
     * @brief Writes a block of data to the underlying stream. The
     * implementation of this function may perform a blocking call.
     * 
     * @param[in] data Const pointer to the data block.
     * @param[in] len Length of the data block to write.
     */
    virtual void _write(const void *data, size_t len) = 0;

    uint8_t *_buf = nullptr;
    size_t _bufSize = 0;
    uint8_t *_cursor = nullptr;
    size_t _remaining = 0;
    SerializationLevel _level = SerializationLevel::PLAIN;

    inline bool _fit(size_t sz) {
        if (sz <= _remaining) {
            return true;
        }
        else {
            flush();
            return sz <= _bufSize;
        }
    }

public:

    /**
     * @param[in] bufferSize Size of the internal buffer. Default size is 1 KiB.
     */
    OutputStreamSerializer(size_t bufferSize = _DEFAULT_BUFFER_SIZE) {
        _buf = new uint8_t[bufferSize];
        _bufSize = bufferSize;
        _cursor = _buf;
        _remaining = bufferSize;
        _level = SerializationLevel::PLAIN;
    }

    OutputStreamSerializer(const OutputStreamSerializer &) = delete;

    OutputStreamSerializer(OutputStreamSerializer &&) = delete;

    ~OutputStreamSerializer() {
        delete[] _buf;
    }

    OutputStreamSerializer & operator=(const OutputStreamSerializer &) = delete;

    OutputStreamSerializer & operator=(OutputStreamSerializer &&) = delete;

    /**
     * @brief Flushes the internal buffer of this serializer.
     * 
     * @return A reference to this object for chaining.
     */
    OutputStreamSerializer & flush() {
        size_t len = _cursor - _buf;
        if (len > 0) {
            _write(_buf, len);
            _cursor = _buf;
            _remaining = _bufSize;
        }
        return *this;
    }

    /**
     * @brief Sets the serialization level.
     * 
     * @param[in] level The serialization level.
     * @return A reference to this object for chaining.
     */
    OutputStreamSerializer & setLevel(SerializationLevel level) {
        _level = level;
        return *this;
    }

    /**
     * @brief Writes a block of data.
     * 
     * @param[in] data Const pointer to the data block.
     * @param[in] len Length of the data block to write.
     * @return A reference to this object for chaining.
     */
    OutputStreamSerializer & put(const void *data, size_t len) {
        if (_fit(len)) {
            memcpy(_cursor, data, len);
            _cursor += len;
            _remaining -= len;
        }
        else {
            _write(data, len);
        }

        return *this;
    }

    /**
     * @brief Writes the bits of the given object.
     * 
     * @param[in] x Const reference to an object.
     * @return A reference to this object for chaining.
     */
    template <
        typename T,
        std::enable_if_t<
            SupportsTrivialSerialization<T> && ! SupportsCustomSerialization<T>,
            int
        > = 0
    >
    OutputStreamSerializer & operator<<(const T &x) {
        if (_fit(sizeof(T))) {
            *((T *) _cursor) = x;
            _cursor += sizeof(T);
            _remaining -= sizeof(T);
        }
        else {
            _write(&x, sizeof(T));
        }
        return *this;
    }

    /**
     * @brief Writes a Serializable object using its writeObject implementation.
     * 
     * @param[in] object Const reference to a serializable object.
     * @return A reference to this object for chaining.
     */
    OutputStreamSerializer & operator<<(const Serializable &object) {
        *this << object.objectCode();
        object.writeObject(*this, _level);
        return *this;
    }

    /**
     * @brief Writes a Serializable object using its writeObject implementation.
     * 
     * @param[in] object Const pointer to a serializable object.
     * @return A reference to this object for chaining.
     */
    OutputStreamSerializer & operator<<(const Serializable *object) {
        if (object == nullptr) {
            *this << (size_t) 0;
        }
        else {
            *this << object->objectCode();
            object->writeObject(*this, _level);
        }
        return *this;
    }
};

/**
 * @brief A serializer used for writing objects to some underlying
 * randomly-accessible and delimited byte range.
*/
class OutputRandomAccessSerializer
:   public OutputStreamSerializer {
private:

    size_t _position = 0;
    size_t _length = -1UL;

protected:

    /**
     * @brief Writes a block of data to the underlying byte range. The
     * implementation of this function may perform a blocking call.
     * 
     * @param[in] position Byte offset (from the beginning) to where the block
     * of data should be written.
     * @param[in] data Const pointer to the data block.
     * @param[in] len Length of the data block to write.
     */
    virtual void _writeAt(size_t position, const void *data, size_t len) = 0;

    /**
     * @return The length of the underlying byte range.
     */
    virtual size_t _getLength() const = 0;

    void _write(const void *data, size_t len) override final {
        if (_position + len > length()) {
            throw OutOfRangeError(
                "Attempt to write beyond the available serialization region"
            );
        }
        _writeAt(_position, data, len);
        _position += len;
    }

public:

    /**
     * @param[in] bufferSize Size of the internal buffer. Default size is 1 KiB.
     */
    OutputRandomAccessSerializer(size_t bufferSize = _DEFAULT_BUFFER_SIZE)
    :   OutputStreamSerializer(bufferSize),
        _position(0),
        _length(-1UL)
    { }

    /**
     * @brief Flushes the internal buffer of this serializer.
     * 
     * @return A reference to this object for chaining.
     */
    OutputRandomAccessSerializer & flush() {
        OutputStreamSerializer::flush();
        return *this;
    }

    /**
     * @brief Sets the serialization level.
     * 
     * @param[in] level The serialization level.
     * @return A reference to this object for chaining.
     */
    OutputRandomAccessSerializer & setLevel(SerializationLevel level) {
        OutputStreamSerializer::setLevel(level);
        return *this;
    }

    /**
     * @return The limit of the underlying byte range.
     */
    size_t length() const {
        if (_length == -1UL) {
            const_cast<OutputRandomAccessSerializer *>(this)->_length = _getLength();
        }
        return _length;
    }

    /**
     * @return The number of bytes between the current position and the end of
     * the underlying byte range.
     */
    size_t remaining() const {
        return length() - _position;
    }

    /**
     * @brief Changes the current position.
     * 
     * @param[in] postition The desired position (from the beginning).
     * @return A reference to this object for chaining.
     */
    OutputRandomAccessSerializer & seekTo(size_t position) {
        if (position > length()) {
            throw OutOfRangeError(
                "Attempt to seek beyond the available serialization region"
            );
        }
        flush();
        _position = position;
        return *this;
    }

    /**
     * @brief Increments the current position.
     * 
     * @param[in] displacement The desired increment in position.
     * @return A reference to this object for chaining.
     */
    OutputRandomAccessSerializer & seekForward(size_t displacement) {
        return seekTo(_position + displacement);
    }

    /**
     * @brief Decrements the current position.
     * 
     * @param[in] displacement The desired decrement in position.
     * @return A reference to this object for chaining.
     */
    OutputRandomAccessSerializer & seekBackward(size_t displacement) {
        return seekTo(_position - displacement);
    }

    /**
     * @brief Writes a block of data.
     * 
     * @param[in] data Const pointer to the data block.
     * @param[in] len Length of the data block to write.
     * @return A reference to this object for chaining.
     */
    OutputRandomAccessSerializer & put(const void *data, size_t len) {
        OutputStreamSerializer::put(data, len);
        return *this;
    }

    /**
     * @brief Writes the bits of the given object.
     * 
     * @param[in] x Const reference to an object.
     * @return A reference to this object for chaining.
     */
    template <
        typename T,
        std::enable_if_t<
            SupportsTrivialSerialization<T> && ! SupportsCustomSerialization<T>,
            int
        > = 0
    >
    OutputRandomAccessSerializer & operator<<(const T &x) {
        OutputStreamSerializer::operator<<<T>(x);
        return *this;
    }

    /**
     * @brief Writes a Serializable object using its writeObject implementation.
     * 
     * @param[in] object Const reference to a serializable object.
     * @return A reference to this object for chaining.
     */
    OutputRandomAccessSerializer & operator<<(const Serializable &object) {
        *this << object.objectCode();
        object.writeObject(*this, _level);
        return *this;
    }

    /**
     * @brief Writes a Serializable object using its writeObject implementation.
     * 
     * @param[in] object Const pointer to a serializable object.
     * @return A reference to this object for chaining.
     */
    OutputRandomAccessSerializer & operator<<(const Serializable *object) {
        if (object == nullptr) {
            *this << (size_t) 0;
        }
        else {
            *this << object->objectCode();
            object->writeObject(*this, _level);
        }
        return *this;
    }
};

/**
 * @brief A serializer used for reading objects from some underlying byte
 * stream.
*/
class InputStreamSerializer {

protected:

    static constexpr size_t _DEFAULT_BUFFER_SIZE = 1024;

    /**
     * @brief Reads a block of data from the underlying stream. The
     * implementation of this function may perform a blocking call.
     * 
     * @param[in] data Pointer to a region of memory used to write data.
     * @param[in] maxLen The maximum number of bytes to read.
     * @return The actual number of bytes read.
     */
    virtual size_t _read(void *data, size_t maxLen) = 0;

    uint8_t *_buf = nullptr;
    size_t _bufSize = 0;
    uint8_t *_cursor = nullptr;
    size_t _available = 0;
    SerializationLevel _level = SerializationLevel::PLAIN;

    inline void _fillBuffer() {
        _available = _read(_buf, _bufSize);
        _cursor = _buf;
    }

    inline void _emptyBuffer() {
        _available = 0;
        _cursor = nullptr;
    }

public:

    /**
     * @param[in] bufferSize Size of the internal buffer. Default size is 1 KiB.
     */
    InputStreamSerializer(size_t bufferSize = _DEFAULT_BUFFER_SIZE) {
        _buf = new uint8_t[bufferSize];
        _bufSize = bufferSize;
        _cursor = nullptr;
        _available = 0;
        _level = SerializationLevel::PLAIN;
    }

    InputStreamSerializer(const InputStreamSerializer &) = delete;

    InputStreamSerializer(InputStreamSerializer &&) = delete;

    ~InputStreamSerializer() {
        delete[] _buf;
    }

    InputStreamSerializer & operator=(const InputStreamSerializer &) = delete;

    InputStreamSerializer & operator=(InputStreamSerializer &&) = delete;

    InputStreamSerializer & setLevel(SerializationLevel level) {
        _level = level;
        return *this;
    }

    /**
     * @brief Reads a block of data.
     * 
     * @param[in] data Pointer to a data block.
     * @param[in] len Length of the data block.
     * @return A reference to this object for chaining.
     */
    InputStreamSerializer & get(void *data, size_t len) {
        if (len <= _available) {
            memcpy(data, _cursor, len);
            _cursor += len;
            _available -= len;
        }
        else {
            memcpy(data, _cursor, _available);
            data = (uint8_t *) data + _available;
            len -= _available;

            if (len < _bufSize) {
                _fillBuffer();
                memcpy(data, _cursor, len);
                _cursor += len;
                _available -= len;
            }
            else {
                _emptyBuffer();
                _read(data, len);
            }
        }
        return *this;
    }

    /**
     * @brief Reads the bits of some object.
     * 
     * @param[out] x Reference to an object.
     * @return A reference to this object for chaining.
     */
    template <
        typename T,
        std::enable_if_t<
            SupportsTrivialSerialization<T> && ! SupportsCustomSerialization<T>,
            int
        > = 0
    >
    InputStreamSerializer & operator>>(T &x) {
        if (sizeof(T) <= _available) {
            x = *((T *) _cursor);
            _cursor += sizeof(T);
            _available -= sizeof(T);
        }
        else {
            void *data = &x;
            size_t len = sizeof(T);

            memcpy(data, _cursor, _available);
            data = (uint8_t *) data + _available;
            len -= _available;

            if (len < _bufSize) {
                _fillBuffer();
                memcpy(data, _cursor, len);
                _cursor += len;
                _available -= len;
            }
            else {
                _emptyBuffer();
                _read(data, len);
            }
        }
        return *this;
    }

    /**
     * @brief Reads a Serializable object using its readObject implementation.
     * 
     * @param[out] object Reference to a serializable object.
     * @return A reference to this object for chaining.
     */
    InputStreamSerializer & operator>>(Serializable &object) {
        size_t code;
        *this >> code;
        object.readObject(*this, _level);
        return *this;
    }

    /**
     * @brief Reads a Serializable object using its readObject implementation.
     * 
     * @param[out] object Reference to a serializable object pointer. If the
     * pointer is a nullptr, this function will try to create an object by
     * looking up and invoking a factory for the detected object type. If the
     * pointer is not a nullptr and a nullptr was serialized, the pointer will
     * be set to null and the existing object will be destroyed.
     * @return A reference to this object for chaining.
     */
    template <
        typename T,
        std::enable_if_t<
            std::is_base_of_v<Serializable, T>,
            int
        > = 0
    >
    InputStreamSerializer & operator>>(T *&object) {
        size_t code;
        *this >> code;
        if (code == 0) {
            if (object != nullptr) {
                delete object;
                object = nullptr;
            }
        }
        else {
            if (object == nullptr) {
                object = Factory::createObject<T>(code);
            }
            object->readObject(*this, _level);
        }
        return *this;
    }
};

/**
 * @brief A serializer used for reading objects from some underlying
 * randomly-accessible and delimited byte range.
*/
class InputRandomAccessSerializer
:   public InputStreamSerializer {
private:

    size_t _position = 0;
    size_t _length = -1UL;

protected:

    /**
     * @brief Reads a block of data from the underlying byte range. The
     * implementation of this function may perform a blocking call.
     * 
     * @param[in] position Byte offset (from the beginning) from where the block
     * of data should be read.
     * @param[in] data Pointer to a region of memory used to write data.
     * @param[in] len The length of data to read.
     */
    virtual void _readAt(size_t position, void *data, size_t len) = 0;

    /**
     * @return The length of the underlying byte range.
     */
    virtual size_t _getLength() const = 0;

    size_t _read(void *data, size_t maxLen) override final {
        size_t l = std::min(maxLen, length() - _position);
        _readAt(_position, data, l);
        _position += l;
        return l;
    }

public:

    /**
     * @param[in] bufferSize Size of the internal buffer. Default size is 1 KiB.
     */
    InputRandomAccessSerializer(size_t bufferSize = _DEFAULT_BUFFER_SIZE)
    :   InputStreamSerializer(bufferSize),
        _position(0),
        _length(-1UL)
    { }

    /**
     * @brief Sets the serialization level.
     * 
     * @param[in] level The serialization level.
     * @return A reference to this object for chaining.
     */
    InputRandomAccessSerializer & setLevel(SerializationLevel level) {
        InputStreamSerializer::setLevel(level);
        return *this;
    }

    /**
     * @return The limit of the underlying byte range.
     */
    size_t length() const {
        if (_length == -1UL) {
            const_cast<InputRandomAccessSerializer *>(this)->_length = _getLength();
        }
        return _length;
    }

    /**
     * @return The number of bytes between the current position and the end of
     * the underlying byte range.
     */
    size_t remaining() const {
        return _length - _position;
    }

    /**
     * @brief Changes the current position.
     * 
     * @param[in] postition The desired position (from the beginning).
     * @return A reference to this object for chaining.
     */
    InputRandomAccessSerializer & seekTo(size_t position) {
        if (position > length()) {
            throw OutOfRangeError(
                "Attempt to seek beyond the available serialization region"
            );
        }
        _emptyBuffer();
        _position = position;
        return *this;
    }

    /**
     * @brief Increments the current position.
     * 
     * @param[in] displacement The desired increment in position.
     * @return A reference to this object for chaining.
     */
    InputRandomAccessSerializer & seekForward(size_t displacement) {
        return seekTo(_position + displacement);
    }

    /**
     * @brief Decrements the current position.
     * 
     * @param[in] displacement The desired decrement in position.
     * @return A reference to this object for chaining.
     */
    InputRandomAccessSerializer & seekBackward(size_t displacement) {
        return seekTo(_position - displacement);
    }

    /**
     * @brief Reads a block of data.
     * 
     * @param[in] data Pointer to a data block.
     * @param[in] len Length of the data block.
     * @return A reference to this object for chaining.
     */
    InputRandomAccessSerializer & get(void *data, size_t len) {
        InputStreamSerializer::get(data, len);
        return *this;
    }

    /**
     * @brief Reads the bits of some object.
     * 
     * @param[out] x Reference to an object.
     * @return A reference to this object for chaining.
     */
    template <
        typename T,
        std::enable_if_t<
            SupportsTrivialSerialization<T> && ! SupportsCustomSerialization<T>,
            int
        > = 0
    >
    InputRandomAccessSerializer & operator>>(T &x) {
        InputStreamSerializer::operator>><T>(x);
        return *this;
    }

    /**
     * @brief Reads a Serializable object using its readObject implementation.
     * 
     * @param[out] object Reference to a serializable object.
     * @return A reference to this object for chaining.
     */
    InputRandomAccessSerializer & operator>>(Serializable &object) {
        size_t code;
        *this >> code;
        object.readObject(*this, _level);
        return *this;
    }

    /**
     * @brief Reads a Serializable object using its readObject implementation.
     * 
     * @param[out] object Reference to a serializable object pointer. If the
     * pointer is a nullptr, this function will try to create an object by
     * looking up and invoking a factory for the detected object type. If the
     * pointer is not a nullptr and a nullptr was serialized, the pointer will
     * be set to null and the existing object will be destroyed.
     * @return A reference to this object for chaining.
     */
    template <
        typename T,
        std::enable_if_t<
            std::is_base_of_v<Serializable, T>,
            int
        > = 0
    >
    InputRandomAccessSerializer & operator>>(T *&object) {
        size_t code;
        *this >> code;
        if (code == 0) {
            if (object != nullptr) {
                delete object;
                object = nullptr;
            }
        }
        else {
            if (object == nullptr) {
                object = Factory::createObject<T>(code);
            }
            object->readObject(*this, _level);
        }
        return *this;
    }
};

}   // namespace spl
