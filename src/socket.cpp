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
#include <cstring>

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
