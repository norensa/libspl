/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <tcp_socket.h>
#include <unistd.h>

using namespace spl;

// TCPSocket ///////////////////////////////////////////////////////////////////

size_t TCPSocket::_MTU = _MAX_MTU;

void TCPSocket::_resizeMTU(size_t oldMTU) {

    if (oldMTU > _MTU) return;

    size_t newMTU = _MTU;
    if (
        (newMTU <= _MAX_MTU && newMTU > 8000)
        || (newMTU <= 512)
    ) {
        --newMTU;
    }
    else {
        newMTU = 512;
    }
    _MTU = newMTU;
}

void TCPSocket::_send(const void *data, size_t len, int flags) {

    size_t maxLen = _INITIAL_SYSCALL_SIZE;

    while (len > 0) {
        ssize_t sent;
        if (len <= maxLen) {
            sent = ::send(_fd, data, len, flags);
        }
        else {
            sent = ::send(_fd, data, maxLen, flags | MSG_MORE);
        }

        if (sent != -1) {
            len -= sent;
            data = (uint8_t *) data + sent;
        }
        else {
            switch (errno) {
            case EAGAIN:
            case EINTR:
                sched_yield();
                break;

            case ECONNRESET:
            case EPIPE:
                throw ConnectionTerminatedError();

            case EMSGSIZE:
                _resizeMTU(maxLen);
                maxLen = _MTU;
                break;

            default:
                throw CustomMessageErrnoRuntimeError("Error sending data");
            }
        }
    }
}

ssize_t TCPSocket::_recv(void *data, size_t len, bool returnOnBlock, int flags) {
    size_t maxLen = _INITIAL_SYSCALL_SIZE;

    size_t requestSize = len;
    while (len > 0) {
        ssize_t recvd = ::recv(
            _fd,
            data,
            len < maxLen ? len : maxLen,
            flags
        );
        if (recvd == 0) {
            if (requestSize - len > 0) break;
            throw ConnectionTerminatedError();
        }
        else if (recvd != -1) {
            len -= recvd;
            data = (uint8_t *) data + recvd;
        }
        else {
            switch (errno) {
            case EAGAIN:
            case EINTR:
                if (returnOnBlock) return requestSize - len;
                else sched_yield();
                break;

            default:
                throw CustomMessageErrnoRuntimeError("Error receiving data");
            }
        }
    }

    return requestSize - len;
}

TCPSocket::TCPSocket(int fd)
:   _fd(fd)
{
    if (_fd == -1) return;

    socklen_t len = sizeof(_addr);
    if (getsockname(_fd, (sockaddr *) &_addr, &len) == -1 || len > sizeof(_addr)) {
        throw CustomMessageErrnoRuntimeError("Error getting socket address");
    }
}

TCPSocket::TCPSocket(const SocketAddress &addr) {
    _fd = socket((int) addr.family, SOCK_STREAM, 0);
    _addr = addr;

    if (_fd == -1) {
        throw CustomMessageErrnoRuntimeError("Error creating socket");
    }

    if (connect(_fd, (sockaddr *) &addr, sizeof(SocketAddress)) == -1) {
        int err = errno;
        ::close(_fd);

        switch (err) {
        case ETIMEDOUT:
            throw ConnectionTimedOutError();

        case ECONNREFUSED:
            throw ConnectionRefusedError();

        case ENETUNREACH:
            throw NetworkUnreachableError();

        default:
            throw CustomMessageErrnoRuntimeError("Error connecting to target");
        }
    }
}

void TCPSocket::close() {
    if (_fd != -1) {
        ::close(_fd);
        _fd = -1;
    }
}

// TCPServerSocket /////////////////////////////////////////////////////////////

TCPServerSocket::TCPServerSocket(in_port_t port, int maxWaitingQueueLength, SocketFamily family)
:   TCPSocket()
{
    _fd = socket((int) family, SOCK_STREAM, 0);

    if (_fd == -1) {
        throw CustomMessageErrnoRuntimeError("Error creating socket");
    }

    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1) { 
        ::close(_fd);
        throw CustomMessageErrnoRuntimeError("Error setting socket options");
    }

    if (family == SocketFamily::IPV4) {
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

    if (listen(_fd, maxWaitingQueueLength) == -1) {
        throw CustomMessageErrnoRuntimeError("Error attempting to listen");
    }

    socklen_t len = sizeof(_addr);
    if (getsockname(_fd, (sockaddr *) &_addr, &len) == -1 || len > sizeof(_addr)) {
        throw CustomMessageErrnoRuntimeError("Error getting socket address");
    }
}

void TCPServerSocket::close() {
    TCPSocket::close();

    for (auto &conn : _connections) {
        if (conn.v != nullptr) delete conn.v;
    }
    _connections.clear();
    _pollfds.clear();
}

TCPSocket TCPServerSocket::accept() {
    SocketAddress addr;
    socklen_t len = sizeof(addr);
    int incoming = ::accept4(_fd, (sockaddr *) &addr, &len, SOCK_NONBLOCK);
    return TCPSocket(incoming, addr);
}

TCPSocket * TCPServerSocket::poll() {
    TCPSocket *ptr = _ready.nonEmpty() ? _ready.dequeue() : nullptr;
    while (ptr == nullptr) {
        poll(
            [this, &ptr] (TCPSocket *conn) {
                if (ptr == nullptr) ptr = conn;
                else _ready.enqueue(conn);
            },
            -1
        );
    }
    return ptr;
}

TCPSocket * TCPServerSocket::pollOrAccept() {
    TCPSocket *ptr = _ready.nonEmpty() ? _ready.dequeue() : nullptr;
    while (ptr == nullptr) {
        pollOrAccept(
            [this, &ptr] (TCPSocket *conn) {
                if (ptr == nullptr) ptr = conn;
                else _ready.enqueue(conn);
            },
            -1
        );
    }
    return ptr;
}
