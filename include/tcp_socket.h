/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <socket.h>
#include <hash_map.h>
#include <hash_set.h>
#include <deque.h>
#include <serialization.h>
#include <poll.h>
#include <vector>

namespace spl {

/**
 * @brief Helper class for creating and managing client TCP connections.
 */
class TCPSocket {
    friend class TCPServerSocket;

protected:

    static const size_t _INITIAL_SYSCALL_SIZE = 64 * 1024;
    static const size_t _MAX_MTU = 8192;

    static size_t _MTU;

    static void _resizeMTU(size_t oldMTU);

    int _fd;
    SocketAddress _addr;

    void _send(const void *data, size_t len, int flags);

    ssize_t _recv(void *data, size_t len, bool returnOnBlock, int flags);

    TCPSocket(int fd, const SocketAddress &addr)
    :   _fd(fd),
        _addr(addr)
    { }

    TCPSocket(int fd);

public:

    /**
     * @brief Construct a new (disconnected) TCPSocket object.
     */
    TCPSocket()
    :   TCPSocket(-1)
    { }

    /**
     * @brief Construct a new TCPSocket object and attempts to connect to the
     * indicated address. If a connection could not be made, one of
     * ConnectionTimedOutError, ConnectionRefusedError, NetworkUnreachableError,
     * or Error (in case of other errors) will be thrown.
     * 
     * @param addr A socket address.
     * @throws ConnectionTimedOutError if the attempt to connect has timed out.
     * @throws ConnectionRefusedError if no one is listening on the specified
     * remote.
     * @throws NetworkUnreachableError if the remote address is unreachable.
     * @throws Error if any other error prevents connection.
     */
    TCPSocket(const SocketAddress &addr);

    /**
     * @brief Construct a new TCPSocket object and attempts to connect to the
     * indicated address. If a connection could not be made, one of
     * ConnectionTimedOutError, ConnectionRefusedError, NetworkUnreachableError,
     * or Error (in case of other errors) will be thrown.
     * 
     * @param ip IP address string.
     * @param port Port number to connect to.
     * @throws ConnectionTimedOutError if the attempt to connect has timed out.
     * @throws ConnectionRefusedError if no one is listening on the specified
     * remote.
     * @throws NetworkUnreachableError if the remote address is unreachable.
     * @throws Error if any other error prevents connection.
     */
    TCPSocket(const char *ip, in_port_t port)
    :   TCPSocket(str_to_addr(ip, port))
    { }

    /**
     * @brief Construct a new TCPSocket object and attempts to connect to the
     * indicated address. If a connection could not be made, one of
     * ConnectionTimedOutError, ConnectionRefusedError, NetworkUnreachableError,
     * or Error (in case of other errors) will be thrown.
     * 
     * @param ip IP address string.
     * @param port Port number to connect to.
     * @throws ConnectionTimedOutError if the attempt to connect has timed out.
     * @throws ConnectionRefusedError if no one is listening on the specified
     * remote.
     * @throws NetworkUnreachableError if the remote address is unreachable.
     * @throws Error if any other error prevents connection.
     */
    TCPSocket(const std::string &ip, in_port_t port)
    :   TCPSocket(str_to_addr(ip, port))
    { }

    /**
     * @brief Construct a new TCPSocket object and attempts to connect to the
     * indicated address. If a connection could not be made, one of
     * ConnectionTimedOutError, ConnectionRefusedError, NetworkUnreachableError,
     * or Error (in case of other errors) will be thrown.
     * 
     * @param addr Socket address string.
     * @throws ConnectionTimedOutError if the attempt to connect has timed out.
     * @throws ConnectionRefusedError if no one is listening on the specified
     * remote.
     * @throws NetworkUnreachableError if the remote address is unreachable.
     * @throws Error if any other error prevents connection.
     */
    TCPSocket(const char *addr)
    :   TCPSocket(str_to_addr(addr))
    { }

    /**
     * @brief Construct a new TCPSocket object and attempts to connect to the
     * indicated address. If a connection could not be made, one of
     * ConnectionTimedOutError, ConnectionRefusedError, NetworkUnreachableError,
     * or Error (in case of other errors) will be thrown.
     * 
     * @param addr Socket address string.
     * @throws ConnectionTimedOutError if the attempt to connect has timed out.
     * @throws ConnectionRefusedError if no one is listening on the specified
     * remote.
     * @throws NetworkUnreachableError if the remote address is unreachable.
     * @throws Error if any other error prevents connection.
     */
    TCPSocket(const std::string &addr)
    :   TCPSocket(str_to_addr(addr))
    { }

    TCPSocket(const TCPSocket &) = delete;

    TCPSocket(TCPSocket &&rhs)
    :   _fd(rhs._fd),
        _addr(std::move(rhs._addr))
    {
        rhs._fd = -1;
    }

    /**
     * @brief Close connection and destroy the TCPSocket object.
     */
    ~TCPSocket() {
        close();
    }

    TCPSocket & operator=(const TCPSocket &) = delete;

    TCPSocket & operator=(TCPSocket &&rhs) {
        if (this != &rhs) {
            close();

            _fd = rhs._fd; rhs._fd = -1;
            _addr = rhs._addr;
        }
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
    void send(const void *data, size_t len) {
        _send(data, len, MSG_NOSIGNAL);
    }

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
    ssize_t recv(void *data, size_t len, bool block = true) {
        return _recv(
            data,
            len,
            ! block,
            block ? MSG_WAITALL : MSG_DONTWAIT
        );
    }

    /**
     * @brief Receives some data without removing it from the buffer. A
     * subsequent call will get the same data. Throws a
     * ConnectionTerminatedError if the other end terminated the connection.
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
    ssize_t peek(void *data, size_t len, bool block = true) {
        return _recv(
            data,
            len,
            ! block,
            block ? (MSG_WAITALL | MSG_PEEK) : (MSG_DONTWAIT | MSG_PEEK)
        );
    }

    /**
     * @brief Closes the underlying file descriptor.
     */
    void close();
};

/**
 * @brief Helper class for managing server-side TCP connections.
 */
class TCPServerSocket
:   public TCPSocket {

private:

    struct PollEntry : pollfd, Hashable {

        PollEntry(int fd)
        :   pollfd({ fd, POLLIN, 0 })
        { }

        size_t hash() const {
            return fd;
        }

        bool operator==(const PollEntry &rhs) const {
            return fd == rhs.fd;
        }
    };

    parallel::HashMap<int, TCPSocket *> _connections;
    parallel::HashSet<PollEntry> _pollfds;
    Deque<TCPSocket *> _ready;

    template <typename F>
    void _pollOrAccept(F f, int timeoutMillis) {

        if (_ready.nonEmpty()) {
            do {
                f(_ready.dequeue());
            } while(_ready.nonEmpty());
            return;
        }

        std::vector<PollEntry> fds(_pollfds.begin(), _pollfds.end());
        if (fds.empty()) {
            throw RuntimeError("No connections to poll");
        }
        int count = ::poll((pollfd *) fds.data(), fds.size(), timeoutMillis);

        for (auto &p : fds) {
            if (count == 0) break;
            if (p.revents == 0) continue;
            --count;

            if (p.fd == _fd) {
                SocketAddress addr;
                socklen_t len = sizeof(addr);
                int newFd;

                if (
                    (p.revents & POLLIN) &&
                    (newFd = accept4(_fd, (sockaddr *) &addr, &len, SOCK_NONBLOCK)) != -1
                ) {
                    auto conn = new TCPSocket(newFd, addr);
                    _connections.put(newFd, conn);
                    f(conn);
                }
            }
            else if ((p.revents & POLLIN)) {
                _pollfds.erase(p.fd);
                f(_connections.get(p.fd));
            }
            else if (
                (p.revents & POLLHUP)
                || (p.revents & POLLRDHUP)
                || (p.revents & POLLERR)
                || (p.revents & POLLNVAL)
            ) {
                _pollfds.erase(p.fd);
                delete _connections.remove(p.fd);
            }
        }
    }

public:

    /**
     * @brief Construct a new TCPServerSocket object.
     * 
     * @param port Port number to listen on.
     * @param maxWaitingQueueLength The maximum number of allowed waiting
     * connections.
     * @param family Socket family to use for the server socket.
     */
    TCPServerSocket(in_port_t port, int maxWaitingQueueLength, SocketFamily family = SocketFamily::IPV4);

    TCPServerSocket(TCPServerSocket &&rhs)
    :   TCPSocket(std::move(rhs)),
        _connections(std::move(rhs._connections)),
        _pollfds(std::move(rhs._pollfds))
    { }

    /**
     * @brief Closes all cached connections and the underlying file descriptor
     * for the server socket, then destroys the TCPServerSocket object.
     */
    ~TCPServerSocket() {
        close();
    }

    TCPServerSocket & operator=(TCPServerSocket &&rhs) {
        if (this != &rhs) {
            TCPSocket::operator=(std::move(rhs));
            _connections = std::move(rhs._connections);
            _pollfds = std::move(rhs._pollfds);
        }
        return *this;
    }

    /**
     * @brief Closes all cached connections and the underlying file descriptor
     * for the server socket.
     */
    void close();

    /**
     * @brief Accepts a new client connection. If no waiting connections are
     * found, this function will block until a connection is available.
     * 
     * @return A TCPSocket object for the new client connection.
     */
    TCPSocket accept();

    /**
     * @return The number of currently cached client connections.
     */
    size_t numConnections() const {
        return _connections.size();
    }

    /**
     * @brief Returns a client connection to the cache.
     * Note: this function is thread safe for concurrent calls on different
     * connections.
     * 
     * @param conn A client connection.
     */
    void returnConnection(TCPSocket *conn) {
        _pollfds.put(conn->_fd);
    }

    /**
     * @brief Closes a client connection and removes it from the cache.
     * Note: this function is thread safe for concurrent calls on different
     * connections.
     * 
     * @param conn A client connection.
     */
    void closeConnection(TCPSocket *conn) {
        _pollfds.erase(conn->_fd);
        delete _connections.remove(conn->_fd);
    }

    /**
     * @brief Detaches a client connection and removes it from the cache.
     * Note: this function is thread safe for concurrent calls on different
     * connections.
     * 
     * @param conn A client connection.
     */
    void detachConnection(TCPSocket *conn) {
        _pollfds.erase(conn->_fd);
        _connections.erase(conn->_fd);
    }

    /**
     * @brief Polls the cached client connections for incoming data.
     * 
     * @param f A functor supporting operator()(TCPSocket *) to call if some
     * client connection has data. The given TCPSocket * should eventually be
     * returned via returnConnection(), or closed via closeConnection().
     * @param timeoutMillis Timeout in milliseconds.
     */
    template <typename F>
    void poll(F f, int timeoutMillis = 10) {
        _pollfds.erase(_fd);
        _pollOrAccept(f, timeoutMillis);
    }

    /**
     * @brief Polls the cached client connections for incoming data. This
     * function blocks until a connection is ready.
     * 
     * @return Pointer to a client connection having data available to read.
     * The TCPSocket * should eventually be returned via returnConnection(), or
     * closed via closeConnection().
     */
    TCPSocket * poll();

    /**
     * @brief Polls the cached client connections for incoming data or accepts a
     * new connection.
     * 
     * @param f A functor supporting operator()(TCPSocket *) to call if some
     * client connection has data. The given TCPSocket * should eventually be
     * returned via returnConnection(), or closed via closeConnection().
     * @param timeoutMillis Timeout in milliseconds.
     */
    template <typename F>
    void pollOrAccept(F f, int timeoutMillis = 10) {
        _pollfds.put(_fd);
        _pollOrAccept(f, timeoutMillis);
    }

    /**
     * @brief Polls the cached client connections for incoming data or accepts a
     * new connection. This function blocks until a connection is ready.
     * 
     * @return Pointer to a client connection having data available to read.
     * The TCPSocket * should eventually be returned via returnConnection(), or
     * closed via closeConnection().
     */
    TCPSocket * pollOrAccept();

    // deleted functions

    void send(const void *data, size_t len) = delete;
    ssize_t recv(void *data, size_t len, bool block) = delete;
    ssize_t peek(void *data, size_t len, bool block) = delete;
};

/**
 * @brief An output stream serializer class using TCPSocket objects.
 */
class OutputTCPSocketSerializer
:   public OutputStreamSerializer
{

private:

    TCPSocket *_socket;
    bool _socketOwner;

protected:

    void _write(const void *data, size_t len) override {
        _socket->send(data, len);
    }

public:

    /**
     * @brief Construct a new OutputTCPSocketSerializer object.
     * 
     * @param socket A TCPSocket object.
     */
    OutputTCPSocketSerializer(TCPSocket &&socket)
    :   _socket(new TCPSocket(std::move(socket))),
        _socketOwner(true)
    { }

    /**
     * @brief Construct a new OutputTCPSocketSerializer object.
     * 
     * @param socket Pointer to a TCPSocket object. The TCPSocket is not
     * destroyed when this class is destructed. The caller should destroy the
     * TCPSocket when needed. This constructor variant allows TCPSocket objects
     * to be used multiple times with different serializers.
     */
    OutputTCPSocketSerializer(TCPSocket *socket)
    :   _socket(socket),
        _socketOwner(false)
    { }

    /**
     * @brief Construct a new OutputTCPSocketSerializer object.
     * 
     * @param socket A TCPSocket object.
     * @param bufferSize Size of the internal serializer buffer.
     */
    OutputTCPSocketSerializer(TCPSocket &&socket, size_t bufferSize)
    :   OutputStreamSerializer(bufferSize),
        _socket(new TCPSocket(std::move(socket))),
        _socketOwner(true)
    { }

    /**
     * @brief Construct a new OutputTCPSocketSerializer object.
     * 
     * @param socket Pointer to a TCPSocket object. The TCPSocket is not
     * destroyed when this class is destructed. The caller should destroy the
     * TCPSocket when needed. This constructor variant allows TCPSocket objects
     * to be used multiple times with different serializers.
     * @param bufferSize Size of the internal serializer buffer.
     */
    OutputTCPSocketSerializer(TCPSocket *socket, size_t bufferSize)
    :   OutputStreamSerializer(bufferSize),
        _socket(socket),
        _socketOwner(false)
    { }

    ~OutputTCPSocketSerializer() {
        if (_socketOwner) delete _socket;
    }
};

/**
 * @brief An input stream serializer class using TCPSocket objects.
 */
class InputTCPSocketSerializer
:   public InputStreamSerializer
{

private:

    TCPSocket *_socket;
    bool _socketOwner;

protected:

    size_t _read(void *data, size_t minLen, size_t maxLen) override {
        size_t l = 0;

        do {
            l += _socket->recv(data, maxLen, false);
            if (l < minLen) sched_yield();
        } while (l < minLen);

        return l;
    }

public:

    /**
     * @brief Construct a new InputTCPSocketSerializer object.
     * 
     * @param socket A TCPSocket object.
     */
    InputTCPSocketSerializer(TCPSocket &&socket)
    :   _socket(new TCPSocket(std::move(socket))),
        _socketOwner(true)
    { }

    /**
     * @brief Construct a new InputTCPSocketSerializer object.
     * 
     * @param socket Pointer to a TCPSocket object. The TCPSocket is not
     * destroyed when this class is destructed. The caller should destroy the
     * TCPSocket when needed. This constructor variant allows TCPSocket objects
     * to be used multiple times with different serializers.
     */
    InputTCPSocketSerializer(TCPSocket *socket)
    :   _socket(socket),
        _socketOwner(false)
    { }

    /**
     * @brief Construct a new InputTCPSocketSerializer object.
     * 
     * @param socket A TCPSocket object.
     * @param bufferSize Size of the internal serializer buffer.
     */
    InputTCPSocketSerializer(TCPSocket &&socket, size_t bufferSize)
    :   InputStreamSerializer(bufferSize),
        _socket(new TCPSocket(std::move(socket))),
        _socketOwner(true)
    { }

    /**
     * @brief Construct a new InputTCPSocketSerializer object.
     * 
     * @param socket Pointer to a TCPSocket object. The TCPSocket is not
     * destroyed when this class is destructed. The caller should destroy the
     * TCPSocket when needed. This constructor variant allows TCPSocket objects
     * to be used multiple times with different serializers.
     * @param bufferSize Size of the internal serializer buffer.
     */
    InputTCPSocketSerializer(TCPSocket *socket, size_t bufferSize)
    :   InputStreamSerializer(bufferSize),
        _socket(socket),
        _socketOwner(false)
    { }

    ~InputTCPSocketSerializer() {
        if (_socketOwner) delete _socket;
    }
};

}   // namespace spl
