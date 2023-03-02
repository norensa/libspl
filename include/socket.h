/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#ifndef LIBSPL_EMBEDDED

#include <sys/socket.h>
#include <netinet/ip.h>
#include <string>
#include <exception.h>

namespace spl {

/**
 * @brief Enumeration for socket families.
 */
enum class SocketFamily : sa_family_t {
    IPV4 = AF_INET,
    IPV6 = AF_INET6,
};

/**
 * @brief Socket address type supporting both IPv4 and IPv6 addresses.
 */
union SocketAddress {
    SocketFamily family;
    sockaddr_in v4;
    sockaddr_in6 v6;
};

/**
 * @brief Splits a socket address string into an IP string and a port.
 * 
 * @param str Socket address string.
 * @param ipStr Buffer to write the IP address.
 * @param ipStrLen The size of the IP address buffer.
 * @param port Reference to an in_port_t variable to write the port number.
 */
void split_socket_address(const char *str, char *ipStr, size_t ipStrLen, in_port_t &port);

/**
 * @brief Splits a socket address string into an IP string and a port.
 * 
 * @param str Socket address string.
 * @param ipStr Buffer to write the IP address.
 * @param ipStrLen The size of the IP address buffer.
 * @param port Reference to an in_port_t variable to write the port number.
 */
inline void split_socket_address(const std::string &str, char *ipStr, size_t ipStrLen, in_port_t &port) {
    return split_socket_address(str.c_str(), ipStr, ipStrLen, port);
}

/**
 * @param str A socket address string.
 * @return True if the the address string is an IPv4 string, false otherwise.
 */
inline bool is_ipv4(const char *str) {
    return strchr(str, '.') != NULL;
}

/**
 * @param str A socket address string.
 * @return True if the the address string is an IPv4 string, false otherwise.
 */
inline bool is_ipv4(const std::string &str) {
    return is_ipv4(str.c_str());
}

/**
 * @param str A socket address string.
 * @return True if the the address string is an IPv6 string, false otherwise.
 */
inline bool is_ipv6(const char *str) {
    return ! is_ipv4(str);
}

/**
 * @param str A socket address string.
 * @return True if the the address string is an IPv6 string, false otherwise.
 */
inline bool is_ipv6(const std::string &str) {
    return ! is_ipv4(str.c_str());
}

/**
 * @param addr A socket address.
 * @return String representation of the given SocketAddress.
 */
std::string addr_to_str(const SocketAddress &addr);

/**
 * @brief Compiles a SocketAddress from the given IPv4 address string and port.
 * 
 * @param ip IPv4 address string.
 * @param port Port number.
 * @return A SocketAddress containing the encoded address and port.
 */
SocketAddress str_to_addr_ipv4(const char *ip, in_port_t port);

/**
 * @brief Compiles a SocketAddress from the given IPv4 address string and port.
 * 
 * @param ip IPv4 address string.
 * @param port Port number.
 * @return A SocketAddress containing the encoded address and port.
 */
inline SocketAddress str_to_addr_ipv4(const std::string &ip, in_port_t port) {
    return str_to_addr_ipv4(ip.c_str(), port);
}

/**
 * @brief Compiles a SocketAddress from the given IPv4 socket address string.
 * 
 * @param str IPv4 socket address string.
 * @return A SocketAddress containing the encoded address and port.
 */
SocketAddress str_to_addr_ipv4(const char *str);

/**
 * @brief Compiles a SocketAddress from the given IPv4 socket address string.
 * 
 * @param str IPv4 socket address string.
 * @return A SocketAddress containing the encoded address and port.
 */
inline SocketAddress str_to_addr_ipv4(const std::string &str) {
    return str_to_addr_ipv4(str.c_str());
}

/**
 * @brief Compiles a SocketAddress from the given IPv6 address string and port.
 * 
 * @param ip IPv6 address string.
 * @param port Port number.
 * @return A SocketAddress containing the encoded address and port.
 */
SocketAddress str_to_addr_ipv6(const char *ip, in_port_t port);

/**
 * @brief Compiles a SocketAddress from the given IPv6 address string and port.
 * 
 * @param ip IPv6 address string.
 * @param port Port number.
 * @return A SocketAddress containing the encoded address and port.
 */
inline SocketAddress str_to_addr_ipv6(const std::string &ip, in_port_t port) {
    return str_to_addr_ipv6(ip.c_str(), port);
}

/**
 * @brief Compiles a SocketAddress from the given IPv6 socket address string.
 * 
 * @param str IPv6 socket address string.
 * @return A SocketAddress containing the encoded address and port.
 */
SocketAddress str_to_addr_ipv6(const char *str);

/**
 * @brief Compiles a SocketAddress from the given IPv6 socket address string.
 * 
 * @param str IPv6 socket address string.
 * @return A SocketAddress containing the encoded address and port.
 */
inline SocketAddress str_to_addr_ipv6(const std::string &str) {
    return str_to_addr_ipv6(str.c_str());
}

/**
 * @brief Compiles a SocketAddress from the given IP address string and port.
 * 
 * @param ip IP address.
 * @param port Port number.
 * @return A SocketAddress containing the encoded address and port.
 */
inline SocketAddress str_to_addr(const char *ip, in_port_t port) {
    return is_ipv4(ip)
        ? str_to_addr_ipv4(ip, port)
        : str_to_addr_ipv6(ip, port);
}

/**
 * @brief Compiles a SocketAddress from the given IP address string and port.
 * 
 * @param ip IP address.
 * @param port Port number.
 * @return A SocketAddress containing the encoded address and port.
 */
inline SocketAddress str_to_addr(const std::string &ip, in_port_t port) {
    return str_to_addr(ip.c_str(), port);
}

/**
 * @brief Compiles a SocketAddress from the given socket address string.
 * 
 * @param str Socket address string.
 * @return A SocketAddress containing the encoded address and port.
 */
inline SocketAddress str_to_addr(const char *str) {
    return is_ipv4(str)
        ? str_to_addr_ipv4(str)
        : str_to_addr_ipv6(str);
}

/**
 * @brief Compiles a SocketAddress from the given socket address string.
 * 
 * @param str Socket address string.
 * @return A SocketAddress containing the encoded address and port.
 */
inline SocketAddress str_to_addr(const std::string &str) {
    return str_to_addr(str.c_str());
}

/**
 * @brief Retrieves the address of this machine.
 * 
 * @param port Port number to include in the generated SocketAddress.
 * @param family Desired SocketFamily for the generated SocketAddress.
 * @return The socket address of this machine.
 */
SocketAddress addr_self(in_port_t port, SocketFamily family = SocketFamily::IPV4);

/**
 * @brief Retrieves the address of this machine.
 * 
 * @param port Port number to include in the generated SocketAddress.
 * @return The IPv4 socket address of this machine.
 */
inline SocketAddress addr_self_ipv4(in_port_t port) {
    return addr_self(port, SocketFamily::IPV4);
}

/**
 * @brief Retrieves the address of this machine.
 * 
 * @param port Port number to include in the generated SocketAddress.
 * @return The IPv6 socket address of this machine.
 */
inline SocketAddress addr_self_ipv6(in_port_t port) {
    return addr_self(port, SocketFamily::IPV6);
}

/**
 * @brief An error to indicate that an attempt to connect has timed out.
 */
class ConnectionTimedOutError
:   public Error
{

public:

    ConnectionTimedOutError()
    :   Error("Connection timed out")
    { }
};

/**
 * @brief An error to indicate that an attempt to connect was refused.
 */
class ConnectionRefusedError
:   public Error
{

public:

    ConnectionRefusedError()
    :   Error("Connection refused")
    { }
};

/**
 * @brief An error to indicate that a target is unreachable.
 */
class NetworkUnreachableError
:   public Error
{

public:

    NetworkUnreachableError()
    :   Error("Network unreachable")
    { }
};

/**
 * @brief An error to indicate that an active connection has been terminated.
 */
class ConnectionTerminatedError
:   public Error
{

public:

    ConnectionTerminatedError()
    :   Error("Connection terminated")
    { }
};

#endif  // LIBSPL_EMBEDDED

}
