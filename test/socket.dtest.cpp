#include <dtest.h>
#include <socket.h>
#include <arpa/inet.h>
#include "test_serializable.cpp"

module("socket-util")
.dependsOn({
    "exception",
});

module("tcp-socket")
.dependsOn({
    "socket-util",
    "exception",
    "parallel::hash-map",
    "parallel::hash-set",
    "deque",
});

module("tcp-socket-serialization")
.dependsOn({
    "tcp-socket",
    "stream-serializer"
});

using namespace spl;

#define TEST_SIZE (10240)

unit("socket-util", "split_socket_address")
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

unit("socket-util", "is_ipv4")
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

unit("socket-util", "is_ipv6")
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

unit("socket-util", "str_to_addr_ipv4")
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

unit("socket-util", "str_to_addr_ipv6")
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

unit("socket-util", "str_to_addr")
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

unit("socket-util", "addr_to_str")
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

dunit("tcp-socket", "tcp-echo")
.workers(1)
.driver([] {
    TCPServerSocket s(0, 128);
    dtest_sendMsg(s.address());

    auto conn = s.accept();
    int data;
    conn.recv(&data, sizeof(data));
    conn.send(&data, sizeof(data));
})
.worker([] {
    SocketAddress addr;
    dtest_recvMsg() >> addr;
    TCPSocket s(addr);

    int data = dtest_random() * 100;
    s.send(&data, sizeof(data));
    int data_echo;
    s.recv(&data_echo, sizeof(data_echo));
    assert(data == data_echo);
});

dunit("tcp-socket", "pollOrAccept(F)")
.workers(4)
.driver([] {
    TCPServerSocket s(0, 128);
    dtest_sendMsg(s.address());

    std::vector<TCPSocket *> conn;
    std::vector<std::vector<int>> data(4);

    bool done = false;
    while (! done) {
        s.pollOrAccept([&done, &conn, &data, &s] (TCPSocket *c) {
            int x;
            try {
                if (c->recv(&x, sizeof(x), false) != sizeof(x)) {
                    s.returnConnection(c);
                    return;
                }
            }
            catch (const ConnectionTerminatedError &) {
                s.closeConnection(c);
                return;
            }

            bool ok = false;

            for (size_t i = 0; i < conn.size(); ++i) {
                if (conn[i] == c) {
                    data[i].push_back(x);
                    ok = true;
                }
            }

            if (! ok) {
                conn.push_back(c);
                data[conn.size() - 1].push_back(x);
            }

            ok = true;
            for (auto &v : data) {
                if (v.size() != 1000) {
                    ok = false;
                    break;
                }
            }

            done = ok;
            s.returnConnection(c);
        });

        assert(s.numConnections() <= 4);
    }
})
.worker([] {
    SocketAddress addr;
    dtest_recvMsg() >> addr;
    TCPSocket s(addr);

    for (auto i = 0; i < 1000; ++i) {
        int data = dtest_random() * 100;
        s.send(&data, sizeof(data));
    }
});

dunit("tcp-socket", "pollOrAccept()")
.workers(4)
.driver([] {
    TCPServerSocket s(0, 128);
    dtest_sendMsg(s.address());

    std::vector<TCPSocket *> conn;
    std::vector<std::vector<int>> data(4);

    bool done = false;
    while (! done) {
        TCPSocket *c = s.pollOrAccept();
        int x;
        try {
            if (c->recv(&x, sizeof(x), false) != sizeof(x)) {
                s.returnConnection(c);
                continue;
            }
        }
        catch (const ConnectionTerminatedError &) {
            s.closeConnection(c);
            continue;
        }

        bool ok = false;

        for (size_t i = 0; i < conn.size(); ++i) {
            if (conn[i] == c) {
                data[i].push_back(x);
                ok = true;
            }
        }

        if (! ok) {
            conn.push_back(c);
            data[conn.size() - 1].push_back(x);
        }

        ok = true;
        for (auto &v : data) {
            if (v.size() != 1000) {
                ok = false;
                break;
            }
        }

        done = ok;
        s.returnConnection(c);

        assert(s.numConnections() <= 4);
    }
})
.worker([] {
    SocketAddress addr;
    dtest_recvMsg() >> addr;
    TCPSocket s(addr);

    for (auto i = 0; i < 1000; ++i) {
        int data = dtest_random() * 100;
        s.send(&data, sizeof(data));
    }
});

dunit("tcp-socket", "poll(F)")
.workers(4)
.driver([] {
    TCPServerSocket s(0, 128);
    dtest_sendMsg(s.address());

    std::vector<TCPSocket *> conn;
    std::vector<std::vector<int>> data(2);

    bool done = false;

    auto f = [&done, &conn, &data, &s] (TCPSocket *c) {
        int x;
        try {
            if (c->recv(&x, sizeof(x), false) != sizeof(x)) {
                s.returnConnection(c);
                return;
            }
        }
        catch (const ConnectionTerminatedError &) {
            return;
        }

        bool ok = false;

        for (size_t i = 0; i < conn.size(); ++i) {
            if (conn[i] == c) {
                data[i].push_back(x);
                ok = true;
            }
        }

        if (! ok) {
            conn.push_back(c);
            data[conn.size() - 1].push_back(x);
        }

        ok = true;
        for (auto &v : data) {
            if (v.size() != 1000) {
                ok = false;
                break;
            }
        }

        done = ok;
        s.returnConnection(c);
    };

    while (! done) {
        if (s.numConnections() < 2) s.pollOrAccept(f);
        else s.poll(f);

        assert(s.numConnections() <= 2);
    }
})
.worker([] {
    SocketAddress addr;
    dtest_recvMsg() >> addr;

    try {
        TCPSocket s(addr);

        for (auto i = 0; i < 1000; ++i) {
            int data = dtest_random() * 100;
            s.send(&data, sizeof(data));
        }
    }
    catch (const ConnectionTimedOutError &) {
        return;
    }
    catch (const ConnectionRefusedError &) {
        return;
    }
    catch (const NetworkUnreachableError &) {
        return;
    }
});

dunit("tcp-socket", "poll()")
.workers(4)
.driver([] {
    TCPServerSocket s(0, 128);
    dtest_sendMsg(s.address());

    std::vector<TCPSocket *> conn;
    std::vector<std::vector<int>> data(2);

    bool done = false;
    while (! done) {
        TCPSocket *c; 
        if (s.numConnections() < 2) c = s.pollOrAccept();
        else c = s.poll();

        int x;
        try {
            if (c->recv(&x, sizeof(x), false) != sizeof(x)) {
                s.returnConnection(c);
                continue;
            }
        }
        catch (const ConnectionTerminatedError &) {
            continue;
        }

        bool ok = false;

        for (size_t i = 0; i < conn.size(); ++i) {
            if (conn[i] == c) {
                data[i].push_back(x);
                ok = true;
            }
        }

        if (! ok) {
            conn.push_back(c);
            data[conn.size() - 1].push_back(x);
        }

        ok = true;
        for (auto &v : data) {
            if (v.size() != 1000) {
                ok = false;
                break;
            }
        }

        done = ok;
        s.returnConnection(c);

        assert(s.numConnections() <= 2);
    }
})
.worker([] {
    SocketAddress addr;
    dtest_recvMsg() >> addr;

    try {
        TCPSocket s(addr);

        for (auto i = 0; i < 1000; ++i) {
            int data = dtest_random() * 100;
            s.send(&data, sizeof(data));
        }
    }
    catch (const ConnectionTimedOutError &) {
        return;
    }
    catch (const ConnectionRefusedError &) {
        return;
    }
    catch (const NetworkUnreachableError &) {
        return;
    }
});

dunit("tcp-socket-serialization", "primitive-types")
.workers(1)
.driver([] {
    TCPServerSocket s(0, 128);
    dtest_sendMsg(s.address());

    int x;
    long y;
    short z;

    auto serializer = InputTCPSocketSerializer(s.accept());
    serializer >> x >> y >> z;
    assert(x == 1);
    assert(y == 2);
    assert(z == 3);
})
.worker([] {
    SocketAddress addr;
    dtest_recvMsg() >> addr;

    int x = 1;
    long y = 2;
    short z = 3;

    auto serializer = OutputTCPSocketSerializer(TCPSocket(addr));
    serializer << x << y << z;
    serializer.flush();
});

dunit("tcp-socket-serialization", "serializable-type")
.workers(1)
.driver([] {
    TCPServerSocket s(0, 128);
    dtest_sendMsg(s.address());

    auto elem = StreamSerializable();

    auto serializer = InputTCPSocketSerializer(s.accept());
    serializer >> elem;
    assert(elem.deserialized());
})
.worker([] {
    SocketAddress addr;
    dtest_recvMsg() >> addr;

    auto elem = StreamSerializable();

    auto serializer = OutputTCPSocketSerializer(TCPSocket(addr));
    serializer << elem;
    serializer.flush();
    assert(elem.serialized());
});

dunit("tcp-socket-serialization", "large-serialization")
.workers(1)
.driver([] {
    TCPServerSocket s(0, 128);
    dtest_sendMsg(s.address());

    auto serializer = InputTCPSocketSerializer(s.accept());
    for (auto i = 0; i < TEST_SIZE; ++i) {
        int x;
        serializer >> x;
        assert(x == i);
    }
})
.worker([] {
    SocketAddress addr;
    dtest_recvMsg() >> addr;

    auto serializer = OutputTCPSocketSerializer(TCPSocket(addr));

    for (auto i = 0; i < TEST_SIZE; ++i) {
        serializer << i;
    }
    serializer.flush();
});

dunit("tcp-socket-serialization", "bulk-serialization")
.workers(1)
.driver([] {
    TCPServerSocket s(0, 128);
    dtest_sendMsg(s.address());

    int *b = new int[TEST_SIZE];
    auto serializer = InputTCPSocketSerializer(s.accept());
    serializer.get(b, TEST_SIZE * sizeof(int));
    delete b;
})
.worker([] {
    SocketAddress addr;
    dtest_recvMsg() >> addr;

    int *a = new int[TEST_SIZE];
    for (auto i = 0; i < TEST_SIZE; ++i) {
        a[i] = dtest_random() * TEST_SIZE;
    }

    auto serializer = OutputTCPSocketSerializer(TCPSocket(addr));
    serializer.put(a, TEST_SIZE * sizeof(int));
    serializer.flush();

    delete a;
});
