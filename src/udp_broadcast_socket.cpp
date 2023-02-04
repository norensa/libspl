/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <udp_broadcast_socket.h>
#include <unistd.h>
#include <hash_map.h>
#include <poll.h>

using namespace spl;

struct SocketAddressHash {
    size_t operator()(const SocketAddress &addr) const {
        if (addr.family == SocketFamily::IPV4) {
            return ((size_t) addr.v4.sin_addr.s_addr << 16) | (size_t) addr.v4.sin_port;
        }
        else {
            return hash_combine(
                ((size_t) addr.v6.sin6_addr.__in6_u.__u6_addr32[0] << 32) | (size_t) addr.v6.sin6_addr.__in6_u.__u6_addr32[1],
                ((size_t) addr.v6.sin6_addr.__in6_u.__u6_addr32[2] << 32) | (size_t) addr.v6.sin6_addr.__in6_u.__u6_addr32[3],
                (size_t) addr.v6.sin6_port
            );
        }
    }
};

struct SocketAddressEqual {
    size_t operator()(const SocketAddress &a, const SocketAddress &b) const {
        if (a.family == SocketFamily::IPV4) {
            return 
                a.v4.sin_addr.s_addr == b.v4.sin_addr.s_addr
                && a.v4.sin_port == b.v4.sin_port;
        }
        else {
            return
                a.v6.sin6_addr.__in6_u.__u6_addr32[0] == b.v6.sin6_addr.__in6_u.__u6_addr32[0]
                && a.v6.sin6_addr.__in6_u.__u6_addr32[1] == b.v6.sin6_addr.__in6_u.__u6_addr32[1]
                && a.v6.sin6_addr.__in6_u.__u6_addr32[2] == b.v6.sin6_addr.__in6_u.__u6_addr32[2]
                && a.v6.sin6_addr.__in6_u.__u6_addr32[3] == b.v6.sin6_addr.__in6_u.__u6_addr32[3]
                && a.v6.sin6_port == b.v6.sin6_port;
        }
    }
};

struct UDPBroadcastSocket::Header {
    len_t len;
    seq_t seq;

    Header() = default;

    Header(len_t type)
    :   len(type),
        seq(0)
    { }

    Header(len_t len, seq_t seq)
    :   len(len),
        seq(seq)
    { }

    len_t type() const {
        return len;
    }

    void type(len_t type) {
        len = type;
    }
};

struct UDPBroadcastSocket::Fragment {
    static constexpr len_t INVALID = (len_t) -1;
    static constexpr len_t START_OF_MSG = (len_t) -2;
    static constexpr len_t END_OF_MSG = (len_t) -3;
    static constexpr len_t RESEND = (len_t) -4;
    static constexpr len_t UNAVAILABLE = (len_t) -5;
    static constexpr len_t SEQUENCE_UPDATE = (len_t) -6;
    static constexpr len_t NORMAL = (len_t) -7;

    Header *header = nullptr;
    const void *body = nullptr;

    Fragment() = default;

    Fragment(Header &header)
    :   header(&header),
        body(nullptr)
    { }

    Fragment(Header &header, const void *body)
    :   header(&header),
        body(body)
    { }

    Fragment(const void *data)
    :   header((Header *) data),
        body((Header *) data + 1)
    { }

    len_t type() const {
        return header == nullptr ? INVALID : header->type();
    }

    size_t totalSize() const {
        return header->type() <= NORMAL
            ? sizeof(Header) + header->len
            : sizeof(Header);
    }

    void invalidate() {
        header = nullptr;
        body = nullptr;
    }

    static seq_t max(seq_t a, seq_t b) {
        if (a <= b) {
            if (a - b <= b - a) return a;
            else return b;
        }
        else {
            if (a - b < b - a) return a;
            else return b;
        }
    }
};

struct UDPBroadcastSocket::FragmentPack {
    size_t datagramSize;
    void *buffer;
    uint8_t *cursor = nullptr;
    size_t size = 0;
    timestamp_t nextSend;

    FragmentPack(size_t datagramSize)
    :   datagramSize(datagramSize),
        buffer(malloc(datagramSize))
    {
        reset();
    }

    FragmentPack(const FragmentPack &) = delete;

    FragmentPack(FragmentPack &&) = delete;

    ~FragmentPack() {
        free(buffer);
    }

    FragmentPack & operator=(const FragmentPack &) = delete;

    FragmentPack & operator=(FragmentPack &&) = delete;

    Fragment first() {
        return Fragment(buffer);
    }

    bool send(int fd, int flags, const SocketAddress &address) const {
        return sendto(
            fd,
            buffer,
            size,
            flags,
            (sockaddr *) &address,
            sizeof(address)
        ) == (ssize_t) size;
    }

    bool send(int fd, int flags, const std::vector<SocketAddress> &addresses) const {
        for (const auto &addr : addresses) {
            while (! send(fd, flags, addr)) {
                switch (errno) {
                case EAGAIN:
                #if EAGAIN != EWOULDBLOCK
                case EWOULDBLOCK:
                #endif
                case EINTR:
                    sched_yield();
                    break;

                default:
                    return false;
                }
            }
        }
        return true;
    }

    bool recv(int fd, int flags, SocketAddress &address) {
        socklen_t addrlen = sizeof(SocketAddress);
        ssize_t l = recvfrom(
            fd,
            buffer,
            datagramSize,
            flags,
            (sockaddr *) &address,
            &addrlen
        );
        if (l > 0) {
            resetCursor();
            size = l;
            return true;
        }
        else {
            return false;
        }
    }

    void resetCursor() {
        cursor = (uint8_t *) buffer;
    }

    void reset() {
        resetCursor();
        size = 0;
    }

    bool available() const {
        return cursor < (uint8_t *) buffer + size;
    }

    bool empty() const {
        return cursor == buffer && size == 0;
    }

    size_t remaining() const {
        return datagramSize - size;
    }

    Fragment pop() {
        Fragment f(cursor);
        cursor += f.totalSize();
        return f;
    }

    bool push(const Fragment &f) {
        if (f.totalSize() <= (size_t) ((uint8_t *) buffer + datagramSize - cursor)) {
            *((Header *) cursor) = *f.header;
            cursor += sizeof(Header);
            size += sizeof(Header);

            if (f.type() <= Fragment::NORMAL) {
                memcpy(cursor, f.body, f.header->len);
                cursor += f.header->len;
                size += f.header->len;
            }

            return true;
        }
        else {
            return false;
        }
    }
};

class UDPBroadcastSocket::Window {

private:

    struct WindowFragment {
        Fragment f;
        FragmentPack *buf;

        void invalidate() {
            f.invalidate();
            buf = nullptr;
        }

        bool occupied() const {
            return buf != nullptr;
        }
    };

    size_t _size;
    WindowFragment *_window;
    static Fragment _invalid;

    void _delete(size_t i) {
        size_t j;

        j = i == 0 ? (_size - 1) : (i - 1);
        while (_window[j].buf == _window[i].buf) {
            _window[j].invalidate();
            j = j == 0 ? (_size - 1) : (j - 1);
        }

        j = (i + 1) % _size;
        while (_window[j].buf == _window[i].buf) {
            _window[j].invalidate();
            j = (j + 1) % _size;
        }

        delete _window[i].buf;
        _window[i].invalidate();
    }

public:

    Window(size_t size)
    :   _size(size),
        _window(new WindowFragment[size])
    {
        for (size_t i = 0; i < size; ++i) {
            _window[i].invalidate();
        }
    }

    Window(const Window &) = delete;

    Window(Window &&) = delete;

    ~Window() {
        for (size_t i = 0; i < _size; ++i) {
            if (_window[i].occupied()) _delete(i);
        }
        delete[] _window;
    }

    Window & operator=(const Window &) = delete;

    Window & operator=(Window &&) = delete;

    bool insert(FragmentPack *pack, bool overwrite = true) {
        bool inserted = false;
        while (pack->available()) {
            Fragment f = pack->pop();
            size_t i = f.header->seq % _size;

            if (_window[i].occupied() && overwrite) _delete(i);

            if (! _window[i].occupied()) {
                _window[i] = { f, pack };
                inserted = true;
            }
        }
        if (inserted) pack->resetCursor();
        else delete pack;
        return inserted;
    }

    size_t size() const {
        return _size;
    }

    Fragment & get(seq_t seq) {
        size_t i = seq % _size;
        if (
            _window[i].f.type() != Fragment::INVALID
            && _window[i].f.header->seq == seq
        ) {
            return _window[i].f;
        }
        else {
            return _invalid;
        }
    }

    const Fragment & get(seq_t seq) const {
        return const_cast<Window *>(this)->get(seq);
    }

    FragmentPack * getPack(seq_t seq) const {
        return _window[seq % _size].buf;
    }

    template <typename F>
    void extract(seq_t start, seq_t end, F f) {
        size_t i;
        FragmentPack *last = nullptr;
        while (start != end) {
            i = start % _size;
            if (_window[i].buf != last) {
                if (_window[i].occupied()) {
                    f(_window[i].buf);
                }
                last = _window[i].buf;
            }
            _window[i].invalidate();
            ++start;
        }

        i = start % _size;
        while (last != nullptr && _window[i].buf == last) {
            _window[i].invalidate();
            i = (i + 1) % _size;
        }
    }
};

UDPBroadcastSocket::Fragment UDPBroadcastSocket::Window::_invalid;

struct UDPBroadcastSocket::SequenceRange {
    seq_t begin;
    seq_t end;
};

class UDPBroadcastSocket::SequenceRangePack {

private:

    FragmentPack *_fp;
    size_t _size;
    size_t _i;
    SequenceRange *_arr;
    bool _allocated;

public:

    SequenceRangePack(FragmentPack *fp)
    :   _fp(fp)
    {
        if (_fp->empty()) {
            Header h(Fragment::RESEND);
            _fp->push(Fragment(h));
            _size = (_fp->remaining() - sizeof(Header)) / sizeof(SequenceRange);
            _i = 0;
            _arr = new SequenceRange[_size];
            _allocated = true;
        }
        else {
            _fp->pop();
            auto f = _fp->pop();
            _size = f.header->len / sizeof(SequenceRange);
            _i = _size;
            _arr = (SequenceRange *) f.body;
            _allocated = false;
        }
    }

    SequenceRangePack(const SequenceRangePack &) = delete;

    SequenceRangePack(SequenceRangePack &&) = delete;

    ~SequenceRangePack() {
        if (_allocated) delete[] _arr;
    }

    SequenceRangePack & operator=(const SequenceRangePack &) = delete;

    SequenceRangePack & operator=(SequenceRangePack &&) = delete;

    const SequenceRange * begin() const {
        return _arr;
    }

    const SequenceRange * end() const {
        return _arr + _i;
    }

    bool push(const SequenceRange &r) {
        if (_i < _size) {
            _arr[_i++] = r;
            return true;
        }
        else {
            return false;
        }
    }

    bool send(int fd, int flags, const SocketAddress &address) {
        if (_i == 0) return true;

        Header h(_i * sizeof(SequenceRange), 0);
        if (! _fp->push(Fragment(h, _arr))) return false;

        if (! _fp->send(fd, flags, address)) return false;

        _fp->reset();
        h.type(Fragment::RESEND);
        _fp->push(Fragment(h));
        _i = 0;

        return true;
    }
};

struct UDPBroadcastSocket::ReceiveBuffer {
    const SocketAddress *address = nullptr;
    FragmentPack *buf = nullptr;
    Fragment fragment;
    uint8_t *cursor = nullptr;
    uint8_t *end = nullptr;

    ReceiveBuffer() = default;

    ReceiveBuffer(const SocketAddress *address, FragmentPack *buf)
    :   address(address),
        buf(buf)
    {
        next();
    }

    bool next() {
        if (buf == nullptr) return false;

        if (! buf->available()) {
            delete buf;
            buf = nullptr;
            fragment.invalidate();
            return false;
        }

        fragment = buf->pop();
        cursor = (uint8_t *) fragment.body;
        end = cursor + fragment.header->len;
        return true;
    }

    bool hasData() {
        return fragment.type() <= Fragment::NORMAL && cursor < end;
    }

    size_t get(void *data, size_t len) {
        size_t l = std::min(len, (size_t) (end - cursor));
        memcpy(data, cursor, l);
        cursor += l;
        return l;
    }
};

struct UDPBroadcastSocket::SendBuffer {
    const void *data = nullptr;
    size_t len = 0;
    FragmentPack *resendRequest = nullptr;

    SendBuffer(const void *data, size_t len)
    :   data(data),
        len(len)
    { }

    SendBuffer(FragmentPack *resendRequest)
    :   resendRequest(resendRequest)
    { }
};

class UDPBroadcastSocket::Stream {

private:

    enum class State : uint8_t {
        UNINITIALIZED,
        TRACKING,
        RECEIVING,
        SPECULATING,
        WAITING_RESEND,
    };

    int _fd;
    SocketAddress _address;

    State _state = State::UNINITIALIZED;
    seq_t _start = 0;
    seq_t _ok = 0;
    seq_t _maxSeq = 0;
    Window _window;
    timestamp_t _lastUpdate;
    uint32_t _timeouts = 0;

    void _reset(seq_t min) {
        if (_state > State::UNINITIALIZED && _start != min) {
            _state = State::TRACKING;

            _window.extract(_start, min, [] (FragmentPack *buf) {
                delete buf;
            });

            seq_t i;
            for (i = min; i != _maxSeq; ++i) {
                if (_window.get(i).type() == Fragment::START_OF_MSG) {
                    _state = State::RECEIVING;
                    _start = i;
                    _ok = i;
                    break;
                }
            }
            if (i == _maxSeq) {
                while (_window.get(i).type() != Fragment::INVALID) ++i;
                _ok = i - 1;
            }

            _window.extract(min, i, [] (FragmentPack *buf) {
                delete buf;
            });
        }
    }

    void _requestResends(FragmentPack *buf) {
        SequenceRangePack request(buf);

        seq_t count = 0;

        if (_ok == _maxSeq) {
            request.push({ _ok + 1, _ok });
            _state = State::SPECULATING;
        }
        else {
            for (seq_t i = _ok + 1; i != _maxSeq && count < 2048; ++i) {
                if (_window.get(i).type() == Fragment::INVALID) {
                    seq_t j = i;
                    while (j != _maxSeq && _window.get(j).type() == Fragment::INVALID && count < 2048) {
                        ++j;
                        ++count;
                    }
                    if (j == _maxSeq) {
                        j = i - 1;
                        if (! request.push({ i, j })) {
                            request.send(_fd, 0, _address);
                            request.push({ i, j });
                        }
                        break;
                    }
                    else {
                        if (! request.push({ i, j })) {
                            request.send(_fd, 0, _address);
                            request.push({ i, j });
                        }
                        i = j;
                    }
                }
            }
            _state = State::WAITING_RESEND;
        }

        request.send(_fd, 0, _address);
    }

public:

    Stream(int fd, const SocketAddress &address, size_t windowSize, const timestamp_t &now)
    :   _fd(fd),
        _address(address),
        _window(windowSize),
        _lastUpdate(now)
    { }

    Stream(const Stream &) = delete;

    Stream(Stream &&rhs) = delete;

    ~Stream() = default;

    Stream & operator=(const Stream &) = delete;

    Stream & operator=(Stream &&rhs) = delete;

    const SocketAddress & address() const {
        return _address;
    }

    void insert(
        FragmentPack *pack,
        parallel::Deque<ReceiveBuffer *> &ready,
        const timestamp_t &now
    ) {
        seq_t seq = pack->first().header->seq;

        if (_state == State::UNINITIALIZED) {
            _window.insert(pack, true);
            _ok = seq - 1;
            _maxSeq = seq;
            _state = State::TRACKING;
        }
        else if (Fragment::max(_ok, seq) == seq) {
            if (_window.insert(pack, false)) {
                _maxSeq = Fragment::max(_maxSeq, seq);
            }
            else {
                return;
            }
        }
        else {
            delete pack;
        }

        _timeouts = 0;

        if (seq != _ok + 1) return;

        _lastUpdate = now;

        seq_t i = _ok + 1;
        for (
            auto type = _window.get(i).type();
            type != Fragment::INVALID;
            type = _window.get(++i).type()
        ) {
            switch (type) {
            case Fragment::START_OF_MSG:
                _state = State::RECEIVING;
                _start = i;
                break;

            case Fragment::END_OF_MSG:
                _state = State::TRACKING;
                _window.extract(_start, i + 1, [this, &ready] (FragmentPack *buf) {
                    ready.enqueue(new ReceiveBuffer(&_address, buf));
                });
                break;

            case Fragment::NORMAL:
                _state = State::RECEIVING;

            default: break;
            }

            _ok = i;
            _maxSeq = Fragment::max(_maxSeq, _ok);
        }
    }

    void updateSequence(FragmentPack *pack) {
        seq_t seq = pack->first().header->seq;

        if (_state == State::UNINITIALIZED) {
            _ok = seq - 1;
            _maxSeq = seq;
            _state = State::TRACKING;
        }
        else {
            _maxSeq = Fragment::max(_maxSeq, seq);
        }
    }

    void updateUnavailable(FragmentPack *pack) {
        if (_state == State::SPECULATING) {
            _state = State::RECEIVING;
            return;
        }

        seq_t max = pack->pop().header->seq;
        while (pack->available()) {
            max = Fragment::max(max, pack->pop().header->seq);
        }
        _maxSeq = Fragment::max(_maxSeq, max);
        _reset(max);
    }

    void check(
        const timestamp_t &now,
        const duration_t &timeout,
        uint32_t maxTimeouts,
        FragmentPack *buf
    ) {
        if (_state >= State::RECEIVING || _ok != _maxSeq) {
            if (now > _lastUpdate + timeout) {
                if (++_timeouts > maxTimeouts) {
                    // _reset(_start + 1);
                    _timeouts = 0;
                }
                else if (_state == State::WAITING_RESEND) {
                    _state = State::RECEIVING;
                }
                _lastUpdate = now;
            }
            else if (_state <= State::RECEIVING && now > _lastUpdate + timeout / 5) {
                _requestResends(buf);
                _lastUpdate = now;
            }
        }
    }
};

// UDPBroadcastSocket //////////////////////////////////////////////////////////

void UDPBroadcastSocket::_free() {
    for (auto &b : _sendQueue) {
        delete b;
    }
    for (auto &b : _recvQueue) {
        if (b->buf != nullptr) delete b->buf;
        delete b;
    }
    if (_receiveBuffer != nullptr) {
        if (_receiveBuffer->buf != nullptr) delete _receiveBuffer->buf;
        delete _receiveBuffer;
    }
}

void UDPBroadcastSocket::_sender() {
    while (_startup) sched_yield();

    Window sendWindow(_sendWindowSize);

    FragmentPack *buf = new FragmentPack(_maxDatagramSize);
    // uint64_t timeout = _sequenceUpdateInterval.count();

    timestamp_t now = clock::now();
    timestamp_t nextSequenceUpdate = now;
    timestamp_t nextCongestionUpdate = now;
    useconds_t congestion = 0;
    seq_t sendCount = 0;
    seq_t resendCount = 0;

    Header header;
    Fragment fragment = Fragment(header);

    auto updateCongestion = [
        &
        // t = (double) _congestionUpdateInterval.count() / 1000
        // maxSend = (seq_t) 0
    ] () mutable {
        // maxSend = std::max(maxSend, sendCount + resendCount);

        if (resendCount > 0) {
            if (sendCount > resendCount) {
                congestion = congestion == 0 ? 30 : (congestion * 1.05);
            }
            else if (sendCount > 0) {
                congestion = congestion == 0 ? 30 : (congestion * 1.5);
            }
        }
        else {
            congestion = congestion <= 10 ? 0 : (congestion / 2);
        }

        std::cout << sendCount << ' ' << resendCount << ' ' << congestion << '\n';

        sendCount = 0;
        resendCount = 0;
    };

    auto send = [&] (FragmentPack *buf) {
        if (congestion > 0 && sendCount % 10 == 0) usleep(congestion);
        if (! buf->send(_fd, 0, _broadcastAddresses)) {
            delete buf;
            return;
        }
        sendWindow.insert(buf);
        ++sendCount;
    };

    auto resend = [&] (FragmentPack *request) {
        buf->reset();

        for (auto [begin, end] : SequenceRangePack(request)) {
            if (end == begin - 1) end = _seq;

            FragmentPack *last = nullptr;
            for (auto i = begin; i != end; ++i) {
                FragmentPack *fp = sendWindow.getPack(i);
                if (fp != nullptr) {
                    if (fp != last) {
                        last = fp;

                        if (now > fp->nextSend) {
                            fp->send(_fd, 0, _broadcastAddresses);
                            fp->nextSend = now + _resendDedupeDuration;
                            ++resendCount;
                        }
                    }
                }
                else {
                    header.type(Fragment::UNAVAILABLE);
                    header.seq = i;

                    if (! buf->push(fragment)) {
                        buf->send(_fd, 0, _broadcastAddresses);
                        buf->reset();
                        buf->push(fragment);
                    }
                }
            }
        }

        if (buf->size > 0) {
            buf->send(_fd, 0, _broadcastAddresses);
        }

        delete request;
    };

    auto updateSequence = [&] () {
        // TODO: find a better way to avoid updating sequence if no
        // data was ever sent
        if (_seq == 0) return;
        buf->reset();
        header.type(Fragment::SEQUENCE_UPDATE);
        header.seq = _seq - 1;
        buf->push(fragment);
        buf->send(_fd, 0, _broadcastAddresses);
    };

    while (! Thread::terminateRequested()) {
        FragmentPack *fp = _sendQueue.tryDequeue(nullptr);
        if (fp != nullptr) {
            if (fp->first().type() == Fragment::RESEND) {
                now = clock::now();
                resend(fp);
                now = clock::now();
            }
            else {
                send(fp);
                now = clock::now();
                nextSequenceUpdate = now + _sequenceUpdateInterval;
            }
        }
        else {
            usleep(10);
            now = clock::now();
        }

        if (now > nextSequenceUpdate) {
            updateSequence();
            nextSequenceUpdate = now + _sequenceUpdateInterval;
        }

        if (now > nextCongestionUpdate) {
            updateCongestion();
            nextCongestionUpdate = now + _congestionUpdateInterval;
        }
    }

    delete buf;
}

void UDPBroadcastSocket::_receiver() {

    while (_startup) sched_yield();

    pollfd pfd { _fd, POLLIN, 0 };
    int timeoutMillis = _timeout.count() / 1000000;

    HashMap<SocketAddress, Stream *, SocketAddressHash, SocketAddressEqual> streams;

    timestamp_t now = clock::now();

    SocketAddress addr;
    FragmentPack *buf = new FragmentPack(_maxDatagramSize);
    int count = 0;

    auto getStream = [this, &streams, &now] (const SocketAddress &addr) {
        auto s = streams.getOr(addr, nullptr);
        if (s == nullptr) {
            s = new Stream(_fd, addr, _recvWindowSize, now);
            streams.put(addr, s);
        }
        return s;
    };

    while (! Thread::terminateRequested()) {
        if (count == 0) {
            count = poll(&pfd, 1, timeoutMillis);
        }

        // receive a fragment pack
        if (count > 0) {
            if (buf->recv(_fd, MSG_DONTWAIT, addr)) {
                now = clock::now();

                switch (buf->first().type()) {
                case Fragment::RESEND:
                    _sendQueue.enqueueFront(buf);
                    buf = new FragmentPack(_maxDatagramSize);
                    break;

                case Fragment::UNAVAILABLE:
                    getStream(addr)->updateUnavailable(buf);
                    break;

                case Fragment::SEQUENCE_UPDATE:
                    getStream(addr)->updateSequence(buf);
                    break;

                default:
                    getStream(addr)->insert(buf, _recvQueue, now);
                    buf = new FragmentPack(_maxDatagramSize);
                    break;
                }
            }
            // receive failed
            else {
                switch (errno) {
                case EAGAIN:
                #if EAGAIN != EWOULDBLOCK
                case EWOULDBLOCK:
                #endif
                case EINTR:
                    break;

                default:
                    // fatal error, terminate thread
                    goto end;
                }

                count = 0;
                now = clock::now();
            }
        }
        else {
            now = clock::now();
        }

        // periodic stream check
        for (auto &n : streams) {
            buf->reset();
            n.v->check(now, _timeout, _maxTimeouts, buf);
        }
    }

end:

    for (auto &n : streams) delete n.v;

    delete buf;
}

UDPBroadcastSocket::UDPBroadcastSocket(
    const std::vector<SocketAddress> &broadcastAddresses,
    in_port_t port,
    SocketFamily family
):  _broadcastAddresses(broadcastAddresses),
    _socketFamily(family),
    _sendQueue(*((parallel::Deque<FragmentPack *> *) &__sendQueue)),
    _recvQueue(*((parallel::Deque<ReceiveBuffer *> *) &__recvQueue)),
    _receiverThread([this] { _receiver(); }),
    _senderThread([this] { _sender(); })
{
    _fd = socket((int) _socketFamily, SOCK_DGRAM, 0);
    if (_fd == -1) {
        throw CustomMessageErrnoRuntimeError("Error creating socket");
    }

    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) == -1) {
        ::close(_fd);
        throw CustomMessageErrnoRuntimeError("Error setting socket options");
    }

    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1) { 
        ::close(_fd);
        throw CustomMessageErrnoRuntimeError("Error setting socket options");
    }

    if (_socketFamily == SocketFamily::IPV4) {
        _addr.v4.sin_family = AF_INET;
        _addr.v4.sin_addr.s_addr = INADDR_ANY;
        _addr.v4.sin_port = htons(port);
    }
    else {
        _addr.v6.sin6_family = AF_INET6;
        _addr.v6.sin6_addr = IN6ADDR_ANY_INIT;
        _addr.v6.sin6_port = htons(port);
    }

    if (bind(_fd, (sockaddr *) &_addr, sizeof(_addr)) == -1) {
        ::close(_fd);
        throw CustomMessageErrnoRuntimeError("Error binding socket to port");
    }

    socklen_t len = sizeof(_addr);
    if (getsockname(_fd, (sockaddr *) &_addr, &len) == -1 || len > sizeof(_addr)) {
        throw CustomMessageErrnoRuntimeError("Error getting socket address");
    }

    _addr = addr_self(ntohs(_addr.v4.sin_port), family);

    _startup = false;
}

void UDPBroadcastSocket::send(const void *data, size_t len) {
    Header header;
    Fragment control = Fragment(header);
    FragmentPack *buf = new FragmentPack(_maxDatagramSize);

    header.type(Fragment::START_OF_MSG);
    header.seq = _seq++;
    buf->push(control);

    while (len > 0) {
        if (buf->remaining() <= sizeof(Header)) {
            buf->resetCursor();
            _sendQueue.enqueue(buf);
            buf = new FragmentPack(_maxDatagramSize);
        }

        header.len = (buf->remaining() < sizeof(Header) + len)
            ? (buf->remaining() - sizeof(Header))
            : len;
        header.seq = _seq++;

        buf->push(Fragment(header, data));
        data = (uint8_t *) data + header.len;
        len -= header.len;
    }

    header.type(Fragment::END_OF_MSG);
    header.seq = _seq++;
    if (! buf->push(control)) {
        buf->resetCursor();
        _sendQueue.enqueue(buf);
        buf = new FragmentPack(_maxDatagramSize);
        buf->push(control);
    }
    buf->resetCursor();
    _sendQueue.enqueue(buf);
}

size_t UDPBroadcastSocket::recv(
    const SocketAddress *&addr,
    void *data,
    size_t len,
    bool block
) {
    size_t count = 0;

    while (count < len) {
        if (_receiveBuffer == nullptr) {
            if (block) {
                _receiveBuffer = _recvQueue.dequeue();
            }
            else {
                _receiveBuffer = _recvQueue.tryDequeue(nullptr);
                if (_receiveBuffer == nullptr) break;
            }
        }

        if (_receiveBuffer->hasData()) {
            size_t l = _receiveBuffer->get(data, len - count);
            count += l;
            data = (uint8_t *) data + l;
        }
        else if (_receiveBuffer->fragment.type() == Fragment::END_OF_MSG) {
            _receiveBuffer->next();
            break;
        }
        else if (! _receiveBuffer->next()) {
            delete _receiveBuffer;
            _receiveBuffer = nullptr;
        }
    }

    if (
        _receiveBuffer != nullptr &&
        ! _receiveBuffer->hasData() &&
        _receiveBuffer->next() &&
        _receiveBuffer->fragment.type() == Fragment::END_OF_MSG
    ) {
        _receiveBuffer->next();
    }

    if (count > 0) addr = _receiveBuffer->address;
    return count;
}

void UDPBroadcastSocket::close() {
    if (_fd != -1) {
        _senderThread.requestTerminate();
        _receiverThread.requestTerminate();
        _senderThread.join();
        _receiverThread.join();
        ::close(_fd);
        _fd = -1;
    }
}
