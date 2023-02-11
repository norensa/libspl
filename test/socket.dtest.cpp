/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <socket.h>
#include <arpa/inet.h>

module("socket")
.dependsOn({
    "exception",
});

using namespace spl;

unit("socket", "split_socket_address")
.body([] {
    char ip4[INET_ADDRSTRLEN];
    char ip6[INET6_ADDRSTRLEN];
    in_port_t port;

    split_socket_address("255.255.255.255xxxx:16666", ip4, INET_ADDRSTRLEN, port);
    assert(strcmp(ip4, "255.255.255.255") == 0);
    assert(port == 16666);

    split_socket_address(std::string("255.255.255.255xxxx:16666"), ip4, INET_ADDRSTRLEN, port);
    assert(strcmp(ip4, "255.255.255.255") == 0);
    assert(port == 16666);

    split_socket_address("::1:16667", ip6, INET6_ADDRSTRLEN, port);
    assert(strcmp(ip6, "::1") == 0);
    assert(port == 16667);

    split_socket_address(std::string("::1:16667"), ip6, INET6_ADDRSTRLEN, port);
    assert(strcmp(ip6, "::1") == 0);
    assert(port == 16667);
});

unit("socket", "is_ipv4")
.body([] {
    assert(is_ipv4("1.1.1.1"));
    assert(is_ipv4("1.1.1.1:1234"));
    assert(! is_ipv4("::1"));
    assert(! is_ipv4("::1:1234"));

    assert(is_ipv4(std::string("1.1.1.1")));
    assert(is_ipv4(std::string("1.1.1.1:1234")));
    assert(! is_ipv4(std::string("::1")));
    assert(! is_ipv4(std::string("::1:1234")));
});

unit("socket", "is_ipv6")
.body([] {
    assert(! is_ipv6("1.1.1.1"));
    assert(! is_ipv6("1.1.1.1:1234"));
    assert(is_ipv6("::1"));
    assert(is_ipv6("::1:1234"));

    assert(! is_ipv6(std::string("1.1.1.1")));
    assert(! is_ipv6(std::string("1.1.1.1:1234")));
    assert(is_ipv6(std::string("::1")));
    assert(is_ipv6(std::string("::1:1234")));
});

unit("socket", "str_to_addr_ipv4")
.body([] {
    sockaddr_in addr_conf_4;
    addr_conf_4.sin_family = AF_INET;
    addr_conf_4.sin_port = htons(40);
    inet_pton(AF_INET, "178.23.65.103", &addr_conf_4.sin_addr);

    SocketAddress addr;

    addr = str_to_addr_ipv4("178.23.65.103", 40);
    assert(addr.family == SocketFamily::IPV4);
    assert(addr.v4.sin_family == AF_INET);
    assert(addr.v4.sin_port == addr_conf_4.sin_port);
    assert(sizeof(addr.v4.sin_addr) == sizeof(addr_conf_4.sin_addr));
    assert(memcmp(&addr.v4.sin_addr, &addr_conf_4.sin_addr, sizeof(addr_conf_4.sin_addr)) == 0);

    addr = str_to_addr_ipv4(std::string("178.23.65.103"), 40);
    assert(addr.family == SocketFamily::IPV4);
    assert(addr.v4.sin_family == AF_INET);
    assert(addr.v4.sin_port == addr_conf_4.sin_port);
    assert(sizeof(addr.v4.sin_addr) == sizeof(addr_conf_4.sin_addr));
    assert(memcmp(&addr.v4.sin_addr, &addr_conf_4.sin_addr, sizeof(addr_conf_4.sin_addr)) == 0);

    addr = str_to_addr_ipv4("178.23.65.103:40");
    assert(addr.family == SocketFamily::IPV4);
    assert(addr.v4.sin_family == AF_INET);
    assert(addr.v4.sin_port == addr_conf_4.sin_port);
    assert(sizeof(addr.v4.sin_addr) == sizeof(addr_conf_4.sin_addr));
    assert(memcmp(&addr.v4.sin_addr, &addr_conf_4.sin_addr, sizeof(addr_conf_4.sin_addr)) == 0);

    addr = str_to_addr_ipv4(std::string("178.23.65.103:40"));
    assert(addr.family == SocketFamily::IPV4);
    assert(addr.v4.sin_family == AF_INET);
    assert(addr.v4.sin_port == addr_conf_4.sin_port);
    assert(sizeof(addr.v4.sin_addr) == sizeof(addr_conf_4.sin_addr));
    assert(memcmp(&addr.v4.sin_addr, &addr_conf_4.sin_addr, sizeof(addr_conf_4.sin_addr)) == 0);
});

unit("socket", "str_to_addr_ipv6")
.body([] {
    sockaddr_in6 addr_conf_6;
    addr_conf_6.sin6_family = AF_INET6;
    addr_conf_6.sin6_port = htons(42);
    inet_pton(AF_INET6, "1:2::3", &addr_conf_6.sin6_addr);

    SocketAddress addr;

    addr = str_to_addr_ipv6("1:2::3", 42);
    assert(addr.family == SocketFamily::IPV6);
    assert(addr.v6.sin6_family == AF_INET6);
    assert(addr.v6.sin6_port == addr_conf_6.sin6_port);
    assert(sizeof(addr.v6.sin6_addr) == sizeof(addr_conf_6.sin6_addr));
    assert(memcmp(&addr.v6.sin6_addr, &addr_conf_6.sin6_addr, sizeof(addr_conf_6.sin6_addr)) == 0);

    addr = str_to_addr_ipv6(std::string("1:2::3"), 42);
    assert(addr.family == SocketFamily::IPV6);
    assert(addr.v6.sin6_family == AF_INET6);
    assert(addr.v6.sin6_port == addr_conf_6.sin6_port);
    assert(sizeof(addr.v6.sin6_addr) == sizeof(addr_conf_6.sin6_addr));
    assert(memcmp(&addr.v6.sin6_addr, &addr_conf_6.sin6_addr, sizeof(addr_conf_6.sin6_addr)) == 0);

    addr = str_to_addr_ipv6("1:2::3:42");
    assert(addr.family == SocketFamily::IPV6);
    assert(addr.v6.sin6_family == AF_INET6);
    assert(addr.v6.sin6_port == addr_conf_6.sin6_port);
    assert(sizeof(addr.v6.sin6_addr) == sizeof(addr_conf_6.sin6_addr));
    assert(memcmp(&addr.v6.sin6_addr, &addr_conf_6.sin6_addr, sizeof(addr_conf_6.sin6_addr)) == 0);

    addr = str_to_addr_ipv6(std::string("1:2::3:42"));
    assert(addr.family == SocketFamily::IPV6);
    assert(addr.v6.sin6_family == AF_INET6);
    assert(addr.v6.sin6_port == addr_conf_6.sin6_port);
    assert(sizeof(addr.v6.sin6_addr) == sizeof(addr_conf_6.sin6_addr));
    assert(memcmp(&addr.v6.sin6_addr, &addr_conf_6.sin6_addr, sizeof(addr_conf_6.sin6_addr)) == 0);
});

unit("socket", "str_to_addr")
.body([] {
    sockaddr_in addr_conf_4;
    addr_conf_4.sin_family = AF_INET;
    addr_conf_4.sin_port = htons(40);
    inet_pton(AF_INET, "178.23.65.103", &addr_conf_4.sin_addr);

    sockaddr_in6 addr_conf_6;
    addr_conf_6.sin6_family = AF_INET6;
    addr_conf_6.sin6_port = htons(42);
    inet_pton(AF_INET6, "1:2::3", &addr_conf_6.sin6_addr);

    SocketAddress addr;

    addr = str_to_addr("178.23.65.103", 40);
    assert(addr.family == SocketFamily::IPV4);
    assert(addr.v4.sin_family == AF_INET);
    assert(addr.v4.sin_port == addr_conf_4.sin_port);
    assert(sizeof(addr.v4.sin_addr) == sizeof(addr_conf_4.sin_addr));
    assert(memcmp(&addr.v4.sin_addr, &addr_conf_4.sin_addr, sizeof(addr_conf_4.sin_addr)) == 0);

    addr = str_to_addr(std::string("178.23.65.103"), 40);
    assert(addr.family == SocketFamily::IPV4);
    assert(addr.v4.sin_family == AF_INET);
    assert(addr.v4.sin_port == addr_conf_4.sin_port);
    assert(sizeof(addr.v4.sin_addr) == sizeof(addr_conf_4.sin_addr));
    assert(memcmp(&addr.v4.sin_addr, &addr_conf_4.sin_addr, sizeof(addr_conf_4.sin_addr)) == 0);

    addr = str_to_addr("178.23.65.103:40");
    assert(addr.family == SocketFamily::IPV4);
    assert(addr.v4.sin_family == AF_INET);
    assert(addr.v4.sin_port == addr_conf_4.sin_port);
    assert(sizeof(addr.v4.sin_addr) == sizeof(addr_conf_4.sin_addr));
    assert(memcmp(&addr.v4.sin_addr, &addr_conf_4.sin_addr, sizeof(addr_conf_4.sin_addr)) == 0);

    addr = str_to_addr(std::string("178.23.65.103:40"));
    assert(addr.family == SocketFamily::IPV4);
    assert(addr.v4.sin_family == AF_INET);
    assert(addr.v4.sin_port == addr_conf_4.sin_port);
    assert(sizeof(addr.v4.sin_addr) == sizeof(addr_conf_4.sin_addr));
    assert(memcmp(&addr.v4.sin_addr, &addr_conf_4.sin_addr, sizeof(addr_conf_4.sin_addr)) == 0);

    addr = str_to_addr("1:2::3", 42);
    assert(addr.family == SocketFamily::IPV6);
    assert(addr.v6.sin6_family == AF_INET6);
    assert(addr.v6.sin6_port == addr_conf_6.sin6_port);
    assert(sizeof(addr.v6.sin6_addr) == sizeof(addr_conf_6.sin6_addr));
    assert(memcmp(&addr.v6.sin6_addr, &addr_conf_6.sin6_addr, sizeof(addr_conf_6.sin6_addr)) == 0);

    addr = str_to_addr(std::string("1:2::3"), 42);
    assert(addr.family == SocketFamily::IPV6);
    assert(addr.v6.sin6_family == AF_INET6);
    assert(addr.v6.sin6_port == addr_conf_6.sin6_port);
    assert(sizeof(addr.v6.sin6_addr) == sizeof(addr_conf_6.sin6_addr));
    assert(memcmp(&addr.v6.sin6_addr, &addr_conf_6.sin6_addr, sizeof(addr_conf_6.sin6_addr)) == 0);

    addr = str_to_addr("1:2::3:42");
    assert(addr.family == SocketFamily::IPV6);
    assert(addr.v6.sin6_family == AF_INET6);
    assert(addr.v6.sin6_port == addr_conf_6.sin6_port);
    assert(sizeof(addr.v6.sin6_addr) == sizeof(addr_conf_6.sin6_addr));
    assert(memcmp(&addr.v6.sin6_addr, &addr_conf_6.sin6_addr, sizeof(addr_conf_6.sin6_addr)) == 0);

    addr = str_to_addr(std::string("1:2::3:42"));
    assert(addr.family == SocketFamily::IPV6);
    assert(addr.v6.sin6_family == AF_INET6);
    assert(addr.v6.sin6_port == addr_conf_6.sin6_port);
    assert(sizeof(addr.v6.sin6_addr) == sizeof(addr_conf_6.sin6_addr));
    assert(memcmp(&addr.v6.sin6_addr, &addr_conf_6.sin6_addr, sizeof(addr_conf_6.sin6_addr)) == 0);
});

unit("socket", "addr_to_str")
.body([] {
    sockaddr_in addr_conf_4;
    addr_conf_4.sin_family = AF_INET;
    addr_conf_4.sin_port = htons(40);
    inet_pton(AF_INET, "178.23.65.103", &addr_conf_4.sin_addr);

    sockaddr_in6 addr_conf_6;
    addr_conf_6.sin6_family = AF_INET6;
    addr_conf_6.sin6_port = htons(42);
    inet_pton(AF_INET6, "1:2::3", &addr_conf_6.sin6_addr);

    SocketAddress addr;
    std::string str;

    addr.v4 = addr_conf_4;
    str = addr_to_str(addr);
    assert(str == "178.23.65.103:40");

    addr.v6 = addr_conf_6;
    str = addr_to_str(addr);
    assert(str == "1:2::3:42");
});
