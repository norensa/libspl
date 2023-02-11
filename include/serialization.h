/*
 * Copyright (c) 2021-2022 Noah Orensa.
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
     */
    virtual void writeObject(OutputStreamSerializer &serializer) const = 0;

    /**
     * @brief Writes this object to a random access serializer. The default
     * implementation delegates this call to the implemented stream
     * serialization.
     * 
     * @param[in] serializer Reference to a random access serializer instance.
     */
    virtual void writeObject(OutputRandomAccessSerializer &serializer) const {
        writeObject((OutputStreamSerializer &) serializer);
    }

    /**
     * @brief Reads this object from a stream serializer.
     * 
     * @param[in] serializer Reference to a stream serializer instance.
     */
    virtual void readObject(InputStreamSerializer &serializer) = 0;

    /**
     * @brief Reads this object from a random access serializer. The default
     * implementation delegates this call to the implemented stream
     * serialization.
     * 
     * @param[in] serializer Reference to a random access serializer instance.
     */
    virtual void readObject(InputRandomAccessSerializer &serializer) {
        readObject((InputStreamSerializer &) serializer);
    }
};

/**
 * @brief A type trait to check if T is trivially serializable. 
 * 
 * @tparam T The type to check.
 */
template <typename T>
struct SupportsTrivialSerialization {
    static constexpr bool value = 
        std::is_copy_assignable<T>::value
        && ! std::is_pointer<T>::value
    ;
};

/**
 * @brief A type trait to check if T implements custom serialization. 
 * 
 * @tparam T The type to check.
 */
template <typename T>
struct SupportsCustomSerialization {
    static constexpr bool value =
        std::is_base_of<Serializable, typename std::remove_pointer<T>::type>::value;
};

/**
 * @brief A type trait to check if T supports serialization (custom or trivial).
 * 
 * @tparam T The type to check.
 */
template <typename T>
struct SupportsSerialization {
    static constexpr bool value =
        SupportsTrivialSerialization<T>::value || SupportsCustomSerialization<T>::value;
};

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

    bool _allocated = false;
    uint8_t *_buf = nullptr;
    size_t _bufSize = 0;
    uint8_t *_cursor = nullptr;
    uint8_t *_lockedCursor = nullptr;
    size_t _remaining = 0;
    SerializationLevel _level = SerializationLevel::PLAIN;
    size_t _totalByteCount = 0;
    size_t _alignment = 1;

private:

    bool _fit(size_t sz) {
        if (sz <= _remaining) {
            return true;
        }
        else if ((_cursor - _buf) % _alignment == 0) {
            flush();
        }
        return sz <= _remaining;
    }

    void _put(const void *data, size_t len) {
        size_t l;
        while (len > 0) {
            l = _cursor - _buf;
            if (l % _alignment == 0 && ((size_t) data) % _alignment == 0 && len % _alignment == 0) {
                if (l > 0) flush();
                _write(data, len);
                return;
            }
            else {
                if (_remaining == 0) flush();
                l = std::min(_remaining, len);
                memcpy(_cursor, data, l);
                _cursor += l;
                _remaining -= l;
                len -= l;
                data = (uint8_t *) data + l;
            }
        }
    }

public:

    /**
     * @param[in] buffer Buffer to use for serialization.
     * @param[in] bufferSize Size of the buffer.
     */
    OutputStreamSerializer(void *buffer, size_t bufferSize)
    :   _buf((uint8_t *) buffer),
        _bufSize(bufferSize),
        _cursor((uint8_t *) buffer),
        _remaining(bufferSize),
        _level(SerializationLevel::PLAIN)
    { }

    /**
     * @param[in] bufferSize Size of the internal buffer. Default size is 1 KiB.
     */
    OutputStreamSerializer(size_t bufferSize = _DEFAULT_BUFFER_SIZE)
    :   OutputStreamSerializer(new uint8_t[bufferSize], bufferSize)
    {
        _allocated = true;
    }

    OutputStreamSerializer(const OutputStreamSerializer &) = delete;

    OutputStreamSerializer(OutputStreamSerializer &&) = delete;

    virtual ~OutputStreamSerializer() {
        if (_allocated) delete[] _buf;
    }

    OutputStreamSerializer & operator=(const OutputStreamSerializer &) = delete;

    OutputStreamSerializer & operator=(OutputStreamSerializer &&) = delete;

    /**
     * @brief Flushes the internal buffer of this serializer.
     * 
     * @return A reference to this object for chaining.
     */
    OutputStreamSerializer & flush() {
        size_t locked = _lockedCursor == nullptr ? 0 : (_cursor - _lockedCursor);
        size_t len = _cursor - _buf - locked;

        if (len > 0) {
            _write(_buf, len);

            if (locked > 0) memmove(_buf, _lockedCursor, locked);
            _cursor = _buf + locked;
            if (_lockedCursor != nullptr) _lockedCursor = _buf;
            _remaining = _bufSize - locked;
        }
        return *this;
    }

    /**
     * @brief Locks the stream cursor in place, preventing any subsequent data
     * from being flushed to the underlying stream before a call to commit() is
     * made.
     * 
     * @return A reference to this object for chaining.
     */
    OutputStreamSerializer & lock() {
        _lockedCursor = _cursor;
        return *this;
    }

    /**
     * @brief Unlocks the stream cursor and commits all serialized data to the
     * underlying stream.
     * 
     * @return A reference to this object for chaining.
     */
    OutputStreamSerializer & commit() {
        _lockedCursor = nullptr;
        return *this;
    }

    /**
     * @return The total number of bytes serialized.
     */
    size_t totalByteCount() const {
        return _totalByteCount;
    }

    /**
     * @return The byte alignment of this serializer.
     */
    size_t alignment() const {
        return _alignment;
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
     * @return The serialization level of this serializer.
     */
    SerializationLevel level() const {
        return _level;
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
            _put(data, len);
        }
        _totalByteCount += len;

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
        typename std::enable_if<
            SupportsTrivialSerialization<T>::value && ! SupportsCustomSerialization<T>::value,
            int
        >::type = 0
    >
    OutputStreamSerializer & operator<<(const T &x) {
        if (_fit(sizeof(T))) {
            *((T *) _cursor) = x;
            _cursor += sizeof(T);
            _remaining -= sizeof(T);
        }
        else {
            _put(&x, sizeof(T));
        }
        _totalByteCount += sizeof(T);
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
        object.writeObject(*this);
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
            object->writeObject(*this);
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
     * @param[in] buffer Buffer to use for serialization.
     * @param[in] bufferSize Size of the buffer.
     */
    OutputRandomAccessSerializer(void *buffer, size_t bufferSize)
    :   OutputStreamSerializer(buffer, bufferSize),
        _position(0),
        _length(-1UL)
    { }

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
     * @brief Locks the stream cursor in place, preventing any subsequent data
     * from being flushed to the underlying stream before a call to commit() is
     * made.
     * 
     * @return A reference to this object for chaining.
     */
    OutputRandomAccessSerializer & lock() {
        OutputStreamSerializer::lock();
        return *this;
    }

    /**
     * @brief Unlocks the stream cursor and commits all serialized data to the
     * underlying stream.
     * 
     * @return A reference to this object for chaining.
     */
    OutputRandomAccessSerializer & commit() {
        OutputStreamSerializer::commit();
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
     * @return The current position.
     */
    size_t tell() const {
        return _position + (_cursor - _buf);
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
        return length() - tell();
    }

    /**
     * @brief Changes the current position.
     * 
     * @param[in] postition The desired position (from the beginning).
     * @return A reference to this object for chaining.
     */
    OutputRandomAccessSerializer & seekTo(size_t position) {
        if (position == _position) return *this;
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
     * @brief Moves the current position.
     * 
     * @param[in] displacement The desired increment/decrement in position.
     * @return A reference to this object for chaining.
     */
    OutputRandomAccessSerializer & seek(ssize_t displacement) {
        ssize_t newPos = _position + displacement;
        if (newPos < 0) {
            throw OutOfRangeError(
                "Attempt to seek beyond the available serialization region"
            );
        }
        return seekTo(newPos);
    }

    /**
     * @brief Moves the current position forward until it is aligned.
     * 
     * @return A reference to this object for chaining.
     */
    OutputRandomAccessSerializer & alignForward() {
        if (_position % _alignment != 0) {
            size_t p = _position / _alignment * _alignment + _alignment;
            seekTo(p);
        }
        return *this;
    }

    /**
     * @brief Moves the current position backward until it is aligned.
     * 
     * @return A reference to this object for chaining.
     */
    OutputRandomAccessSerializer & alignBackward() {
        if (_position % _alignment != 0) {
            size_t p = _position / _alignment * _alignment;
            seekTo(p);
        }
        return *this;
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
        typename std::enable_if<
            SupportsTrivialSerialization<T>::value && ! SupportsCustomSerialization<T>::value,
            int
        >::type = 0
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
        object.writeObject(*this);
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
            object->writeObject(*this);
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
     * @param[in] minLen The minimum number of bytes required.
     * @param[in] maxLen The maximum number of bytes to read.
     * @return The actual number of bytes read.
     */
    virtual size_t _read(void *data, size_t minLen, size_t maxLen) = 0;

    bool _allocated = false;
    uint8_t *_buf = nullptr;
    size_t _bufSize = 0;
    uint8_t *_cursor = nullptr;
    size_t _available = 0;
    SerializationLevel _level = SerializationLevel::PLAIN;
    size_t _totalByteCount = 0;
    size_t _alignment = 1;

protected:

    void _emptyBuffer() {
        _available = 0;
        _cursor = nullptr;
    }

private:

    void _fillBuffer(size_t minLen = 0) {
        _available = _read(_buf, minLen, _bufSize);
        _totalByteCount += _available;
        _cursor = _buf;
    }

    void _get(void *data, size_t len) {
        size_t l;
        while (len > 0) {
            if (len >= _bufSize && _available == 0 && ((size_t) data) % _alignment == 0 && len % _alignment == 0) {
                _read(data, len, len);
                _totalByteCount += len;
                return;
            }
            else {
                if (_available == 0) _fillBuffer(1);
                l = std::min(_available, len);
                memcpy(data, _cursor, l);
                _cursor += l;
                _available -= l;
                len -= l;
                data = (uint8_t *) data + l;
            }
        }
    }

public:

    /**
     * @param[in] buffer Buffer to use for serialization.
     * @param[in] bufferSize Size of the buffer.
     */
    InputStreamSerializer(void *buffer, size_t bufferSize)
    :   _buf((uint8_t *) buffer),
        _bufSize(bufferSize),
        _level(SerializationLevel::PLAIN)
    { }

    /**
     * @param[in] bufferSize Size of the internal buffer. Default size is 1 KiB.
     */
    InputStreamSerializer(size_t bufferSize = _DEFAULT_BUFFER_SIZE)
    :   InputStreamSerializer(new uint8_t[bufferSize], bufferSize)
    {
        _allocated = true;
    }

    InputStreamSerializer(const InputStreamSerializer &) = delete;

    InputStreamSerializer(InputStreamSerializer &&) = delete;

    virtual ~InputStreamSerializer() {
        if (_allocated) delete[] _buf;
    }

    InputStreamSerializer & operator=(const InputStreamSerializer &) = delete;

    InputStreamSerializer & operator=(InputStreamSerializer &&) = delete;

    /**
     * @return The total number of bytes read. Note that this number may include
     * additional bytes read in internal buffers but not yet read by the user.
     */
    size_t totalByteCount() const {
        return _totalByteCount;
    }

    /**
     * @return The byte alignment of this serializer.
     */
    size_t alignment() const {
        return _alignment;
    }

    /**
     * @brief Sets the serialization level.
     * 
     * @param[in] level The serialization level.
     * @return A reference to this object for chaining.
     */
    InputStreamSerializer & setLevel(SerializationLevel level) {
        _level = level;
        return *this;
    }

    /**
     * @return The serialization level of this serializer.
     */
    SerializationLevel level() const {
        return _level;
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
            _get(data, len);
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
        typename std::enable_if<
            SupportsTrivialSerialization<T>::value && ! SupportsCustomSerialization<T>::value,
            int
        >::type = 0
    >
    InputStreamSerializer & operator>>(T &x) {
        if (sizeof(T) <= _available) {
            x = *((T *) _cursor);
            _cursor += sizeof(T);
            _available -= sizeof(T);
        }
        else {
            _get(&x, sizeof(T));
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
        object.readObject(*this);
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
        typename std::enable_if<
            std::is_base_of<Serializable, T>::value,
            int
        >::type = 0
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
            object->readObject(*this);
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

    size_t _read(void *data, size_t minLen, size_t maxLen) override final {

        size_t r = length() - _position;

        if (r < minLen) {
            throw OutOfRangeError(
                "Attempt to read beyond the available serialization region"
            );
        }

        size_t l = std::min(maxLen, r);

        _readAt(_position, data, l);
        _position += l;
        return l;
    }

public:

    /**
     * @param[in] buffer Buffer to use for serialization.
     * @param[in] bufferSize Size of the buffer.
     */
    InputRandomAccessSerializer(void *buffer, size_t bufferSize)
    :   InputStreamSerializer(buffer, bufferSize),
        _position(0),
        _length(-1UL)
    { }

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
     * @return The current position.
     */
    size_t tell() const {
        return _position - _available;
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
        return length() - tell();
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
     * @brief Moves the current position.
     * 
     * @param[in] displacement The desired increment/decrement in position.
     * @return A reference to this object for chaining.
     */
    InputRandomAccessSerializer & seek(ssize_t displacement) {
        ssize_t newPos = _position + displacement;
        if (newPos < 0) {
            throw OutOfRangeError(
                "Attempt to seek beyond the available serialization region"
            );
        }
        return seekTo(newPos);
    }

    /**
     * @brief Moves the current position forward until it is aligned.
     * 
     * @return A reference to this object for chaining.
     */
    InputRandomAccessSerializer & alignForward() {
        if (_position % _alignment != 0) {
            size_t p = _position / _alignment * _alignment + _alignment;
            seekTo(p);
        }
        return *this;
    }

    /**
     * @brief Moves the current position forward until it is aligned.
     * 
     * @return A reference to this object for chaining.
     */
    InputRandomAccessSerializer & alignBackward() {
        if (_position % _alignment != 0) {
            size_t p = _position / _alignment * _alignment;
            seekTo(p);
        }
        return *this;
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
        typename std::enable_if<
            SupportsTrivialSerialization<T>::value && ! SupportsCustomSerialization<T>::value,
            int
        >::type = 0
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
        object.readObject(*this);
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
        typename std::enable_if<
            std::is_base_of<Serializable, T>::value,
            int
        >::type = 0
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
            object->readObject(*this);
        }
        return *this;
    }
};

}   // namespace spl
