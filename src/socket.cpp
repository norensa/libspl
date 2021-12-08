/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <socket.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <unistd.h>
#include <sstream>

using namespace spl;

void spl::split_socket_address(const char *str, char *ipStr, size_t ipStrLen, in_port_t &port) {
    const char *colon = strrchr(str, ':');

    if (colon == nullptr) {
        throw InvalidArgument("Error parsing socket address '", str, '\'');
    }

    size_t sz = std::min((size_t) (colon - str), ipStrLen - 1);
    strncpy(ipStr, str, sz);
    ipStr[sz] = '\0';
    port = atoi(colon + 1);
}

std::string spl::addr_to_str(const SocketAddress &addr) {
    std::stringstream s;

    if (addr.family == SocketFamily::IPV4) {
        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.v4.sin_addr, ipstr, INET_ADDRSTRLEN);
        s << ipstr;
        s << ':';
        s << ntohs(addr.v4.sin_port);
    }
    else {
        char ipstr[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &addr.v6.sin6_addr, ipstr, INET6_ADDRSTRLEN);
        s << ipstr;
        s << ':';
        s << ntohs(addr.v6.sin6_port);
    }

    return s.str();
}

SocketAddress spl::str_to_addr_ipv4(const char *ip, in_port_t port) {
    SocketAddress addr;

    addr.v4.sin_family = AF_INET;
    addr.v4.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr.v4.sin_addr) == -1) {
        throw InvalidArgument("Error parsing IP address '", ip, '\'');
    }

    return addr;
}

SocketAddress spl::str_to_addr_ipv4(const char *str) {
    char ip[INET_ADDRSTRLEN + 6];
    in_port_t port;
    split_socket_address(str, ip, INET_ADDRSTRLEN + 6, port);
    return str_to_addr_ipv4(ip, port);
}

SocketAddress spl::str_to_addr_ipv6(const char *ip, in_port_t port) {
    SocketAddress addr;

    addr.v6.sin6_family = AF_INET6;
    addr.v6.sin6_port = htons(port);
    if (inet_pton(AF_INET6, ip, &addr.v6.sin6_addr) == -1) {
        throw InvalidArgument("Error parsing IP address '", ip, '\'');
    }

    return addr;
}

SocketAddress spl::str_to_addr_ipv6(const char *str) {
    char ip[INET6_ADDRSTRLEN + 6];
    in_port_t port;
    split_socket_address(str, ip, INET6_ADDRSTRLEN + 6, port);
    return str_to_addr_ipv6(ip, port);
}

SocketAddress spl::addr_self(in_port_t port, SocketFamily family) {

    ifaddrs *ifaddr;

    if (getifaddrs(&ifaddr)) {
        throw CustomMessageErrnoRuntimeError("Error getting network interface information");
    }

    SocketAddress self;
    bool found = false;
    for (auto ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (
            (ifa->ifa_flags & IFF_LOOPBACK) == 0 &&             // not loopback
            ifa->ifa_addr->sa_family == (sa_family_t) family    // and matches family
        ) {
            if (ifa->ifa_addr->sa_family == AF_INET) {
                self.v4 = *((sockaddr_in *) ifa->ifa_addr);
                found = true;
            }
            else if (ifa->ifa_addr->sa_family == AF_INET6) {
                self.v6 = *((sockaddr_in6 *) ifa->ifa_addr);
                found = true;
            }
            break;
        }
    }

    freeifaddrs(ifaddr);

    if (found) {
        if (self.family == SocketFamily::IPV4) {
            self.v4.sin_port = htons(port);
        }
        else {
            self.v6.sin6_port = htons(port);
        }
        return self;
    }
    else {
        throw Error("Failed to find own interface");
    }
}

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

    for (auto &[_, conn] : _connections) {
        if (conn != nullptr) delete conn;
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
