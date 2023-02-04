/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <socket.h>
#include <stdint.h>
#include <hash.h>
#include <chrono>
#include <thread.h>
#include <deque.h>
#include <mutex>
#include <condition_variable>

namespace spl {

class UDPBroadcastSocket {

private:

    using clock = std::chrono::high_resolution_clock;
    using timestamp_t = clock::time_point;
    using duration_t = clock::duration;

    typedef uint32_t seq_t;
    typedef uint16_t len_t;

    struct Header;
    struct Fragment;
    struct FragmentPack;
    class Window;
    struct SequenceRange;
    struct SequenceRangePack;
    struct ReceiveBuffer;
    struct SendBuffer;
    class Stream;

    std::vector<SocketAddress> _broadcastAddresses;

    SocketFamily _socketFamily;
    size_t _maxDatagramSize = 508;
    size_t _sendWindowSize = 256 * 1024;
    size_t _recvWindowSize = 256 * 1024;
    duration_t _timeout = std::chrono::milliseconds(100);
    uint32_t _maxTimeouts = 10;
    duration_t _resendDedupeDuration = std::chrono::milliseconds(5);
    duration_t _sequenceUpdateInterval = std::chrono::milliseconds(50);
    duration_t _congestionUpdateInterval = std::chrono::milliseconds(50);

    SocketAddress _addr;
    int _fd;

    seq_t _seq = 0;
    parallel::Deque<void *> __sendQueue;
    parallel::Deque<FragmentPack *> &_sendQueue;

    parallel::Deque<void *> __recvQueue;
    parallel::Deque<ReceiveBuffer *> &_recvQueue;
    ReceiveBuffer *_receiveBuffer = nullptr;

    volatile bool _startup = true;
    Thread _receiverThread;
    Thread _senderThread;

    void _free();

    void _sender();

    void _receiver();

public:

    UDPBroadcastSocket(
        const std::vector<SocketAddress> &broadcastAddresses,
        in_port_t port,
        SocketFamily family = SocketFamily::IPV4
    );

    UDPBroadcastSocket(
        in_port_t port,
        SocketFamily family = SocketFamily::IPV4
    ):  UDPBroadcastSocket({ }, port, family)
    { }

    UDPBroadcastSocket(const UDPBroadcastSocket &) = delete;

    UDPBroadcastSocket(UDPBroadcastSocket &&rhs) = delete;

    ~UDPBroadcastSocket() {
        close();
        _free();
    }

    UDPBroadcastSocket & operator=(const UDPBroadcastSocket &) = delete;

    UDPBroadcastSocket & operator=(UDPBroadcastSocket &&rhs) = delete;

    const std::vector<SocketAddress> & broadcastAddresses() const {
        return _broadcastAddresses;
    }

    UDPBroadcastSocket & broadcastAddresses(const std::vector<SocketAddress> &addresses) {
        _broadcastAddresses = addresses;
        return *this;
    }

    size_t maxDatagramSize() const {
        return _maxDatagramSize;
    }

    UDPBroadcastSocket & maxDatagramSize(size_t size) {
        _maxDatagramSize = size;
        return *this;
    }

    /**
     * @return The address of this socket.
     */
    const SocketAddress & address() const {
        return _addr;
    }

    /**
     * @brief Sends a block of data. Throws a ConnectionTerminatedError if the
     * other end terminated the connection.
     * 
     * @param data Pointer to data to be sent.
     * @param len Length of the data to be sent.
     * @throws ConnectionTerminatedError if the other end terminated the
     * connection.
     * @throws Error if an unexpected error prevents sending.
     * @return True if the operation was successful, false if the operation
     * could not be completed.
     */
    void send(const void *data, size_t len);

    /**
     * @brief Receives some data. Throws a ConnectionTerminatedError if the
     * other end terminated the connection.
     * 
     * @param data Pointer to a buffer for writing the data.
     * @param len Maximum length of data to receive.
     * @param block If true, the function will block until len bytes of data are
     * received.
     * @throws ConnectionTerminatedError if the other end terminated the
     * connection.
     * @throws Error if an unexpected error prevents receiving.
     * @return Size of the data actually received, if successful. If no more
     * data can be retrieved, 0 is returned.
     */
    size_t recv(const SocketAddress *&addr, void *data, size_t len, bool block = true);

    /**
     * @brief Receives some data. Throws a ConnectionTerminatedError if the
     * other end terminated the connection.
     * 
     * @param data Pointer to a buffer for writing the data.
     * @param len Maximum length of data to receive.
     * @param block If true, the function will block until len bytes of data are
     * received.
     * @throws ConnectionTerminatedError if the other end terminated the
     * connection.
     * @throws Error if an unexpected error prevents receiving.
     * @return Size of the data actually received, if successful. If no more
     * data can be retrieved, 0 is returned.
     */
    size_t recv(void *data, size_t len, bool block = true) {
        const SocketAddress *addr;
        return recv(addr, data, len, block);
    }

    /**
     * @brief Closes the underlying file descriptor.
     */
    void close();
};

}
