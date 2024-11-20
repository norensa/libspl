/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <tcp_socket.h>
#include "test_serializable.cpp"

module("tcp-socket")
.dependsOn({
    "socket",
    "exception",
    "parallel::hash-map",
    "parallel::hash-set",
    "deque",
});

module("tcp-socket-serializer")
.dependsOn({
    "tcp-socket",
    "stream-serializer"
});

using namespace spl;

#define TEST_SIZE (10240)

dunit("tcp-socket", "echo")
.workers(1)
.driver([] {
    TCPServerSocket s(0, 128);
    dtest_send_msg(s.address());

    auto conn = s.accept();
    int data;
    conn.recv(&data, sizeof(data));
    conn.send(&data, sizeof(data));
})
.worker([] {
    SocketAddress addr;
    dtest_recv_msg(addr);
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
    dtest_send_msg(s.address());

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
    dtest_recv_msg(addr);
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
    dtest_send_msg(s.address());

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
    dtest_recv_msg(addr);
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
    dtest_send_msg(s.address());

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
    dtest_recv_msg(addr);

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
    dtest_send_msg(s.address());

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
    dtest_recv_msg(addr);

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

dunit("tcp-socket-serializer", "primitive-types")
.workers(1)
.driver([] {
    TCPServerSocket s(0, 128);
    dtest_send_msg(s.address());

    int x;
    long y;
    short z;

    InputTCPSocketSerializer serializer(s.accept());
    serializer >> x >> y >> z;
    assert(x == 1);
    assert(y == 2);
    assert(z == 3);
})
.worker([] {
    SocketAddress addr;
    dtest_recv_msg(addr);

    int x = 1;
    long y = 2;
    short z = 3;

    OutputTCPSocketSerializer serializer{TCPSocket(addr)};
    serializer << x << y << z;
    serializer.flush();
});

dunit("tcp-socket-serializer", "serializable-type")
.workers(1)
.driver([] {
    TCPServerSocket s(0, 128);
    dtest_send_msg(s.address());

    auto elem = StreamSerializable();

    InputTCPSocketSerializer serializer(s.accept());
    serializer >> elem;
    assert(elem.deserialized());
})
.worker([] {
    SocketAddress addr;
    dtest_recv_msg(addr);

    auto elem = StreamSerializable();

    OutputTCPSocketSerializer serializer{TCPSocket(addr)};
    serializer << elem;
    serializer.flush();
    assert(elem.serialized());
});

dunit("tcp-socket-serializer", "large-serialization")
.workers(1)
.driver([] {
    TCPServerSocket s(0, 128);
    dtest_send_msg(s.address());

    InputTCPSocketSerializer serializer(s.accept());
    for (auto i = 0; i < TEST_SIZE; ++i) {
        int x;
        serializer >> x;
        assert(x == i);
    }
})
.worker([] {
    SocketAddress addr;
    dtest_recv_msg(addr);

    OutputTCPSocketSerializer serializer{TCPSocket(addr)};

    for (auto i = 0; i < TEST_SIZE; ++i) {
        serializer << i;
    }
    serializer.flush();
});

dunit("tcp-socket-serializer", "bulk-serialization")
.workers(1)
.driver([] {
    TCPServerSocket s(0, 128);
    dtest_send_msg(s.address());

    int *b = new int[TEST_SIZE];
    InputTCPSocketSerializer serializer(s.accept());
    serializer.get(b, TEST_SIZE * sizeof(int));
    delete[] b;
})
.worker([] {
    SocketAddress addr;
    dtest_recv_msg(addr);

    int *a = new int[TEST_SIZE];
    for (auto i = 0; i < TEST_SIZE; ++i) {
        a[i] = dtest_random() * TEST_SIZE;
    }

    OutputTCPSocketSerializer serializer{TCPSocket(addr)};
    serializer.put(a, TEST_SIZE * sizeof(int));
    serializer.flush();

    delete[] a;
});
