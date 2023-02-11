/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <serialization.h>

using namespace spl;

class MemoryInputStreamSerializer
:   public InputStreamSerializer
{
private:

    uint8_t *_mem;
    size_t _len = 0;
    size_t _pos = 0;

protected:

    size_t _read(void *data, size_t minLen, size_t maxLen) override {
        size_t l = std::min(maxLen, _len - _pos);
        if (l < minLen) throw OutOfRangeError();
        memcpy(data, _mem + _pos, l);
        _pos += l;
        return l;
    }

public:

    MemoryInputStreamSerializer(uint8_t *mem, size_t len)
    :   _mem(mem),
        _len(len),
        _pos(0)
    { }
};

class MemoryOutputStreamSerializer
:   public OutputStreamSerializer
{
private:

    uint8_t *_mem;
    size_t _pos = 0;

protected:

    void _write(const void *data, size_t len) override {
        memcpy(_mem + _pos, data, len);
        _pos += len;
    }

public:

    MemoryOutputStreamSerializer()
    :   _mem(new uint8_t[1024 * 1024]),
        _pos(0)
    { }

    ~MemoryOutputStreamSerializer() {
        delete[] _mem;
    }

    MemoryInputStreamSerializer * toInput() {
        return new MemoryInputStreamSerializer(_mem, _pos);
    }
};

class MemoryInputRandomAccessSerializer
:   public InputRandomAccessSerializer
{
private:

    uint8_t *_mem;
    size_t _len = 0;

protected:

    void _readAt(size_t position, void *data, size_t len) override {
        memcpy(data, _mem + position, len);
    }

    size_t _getLength() const override {
        return _len;
    }

public:

    MemoryInputRandomAccessSerializer(uint8_t *mem, size_t len)
    :   _mem(mem),
        _len(len)
    { }
};

class MemoryOutputRandomAccessSerializer
:   public OutputRandomAccessSerializer
{
private:

    uint8_t *_mem;

protected:

    void _writeAt(size_t position, const void *data, size_t len) override {
        memcpy(_mem + position, data, len);
    }

    size_t _getLength() const override {
        return 1024 * 1024;
    }

public:

    MemoryOutputRandomAccessSerializer()
    :   _mem(new uint8_t[1024 * 1024])
    { }

    ~MemoryOutputRandomAccessSerializer() {
        delete[] _mem;
    }

    MemoryInputRandomAccessSerializer * toInput() {
        return new MemoryInputRandomAccessSerializer(_mem, 1024 * 1024);
    }
};
