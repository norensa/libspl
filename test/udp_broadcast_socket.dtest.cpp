#define DTEST_DISABLE_ALL
#include <dtest.h>
#include <udp_broadcast_socket.h>

module("udp-broadcast-socket")
.dependsOn({
    "socket",
    "hash-map",
    "parallel::deque",
    "thread",
});

using namespace spl;

#define SMALL_MESSAGE_SIZE 128
#define MEDIUM_MESSAGE_SIZE 1500
#define LARGE_MESSAGE_SIZE 102400
#define MANY_MESSAGES 1000

struct RandomizedMessage {
    uint8_t *data;
    size_t size;

    RandomizedMessage(size_t size, bool generate = true)
    :   data(new uint8_t[size]),
        size(size)
    {
        if (generate) {
            uint8_t sum = 0;
            for (size_t i = 0; i < size - 1; ++i) {
                data[i] = dtest_random() * 255;
                sum += data[i];
            }
            data[size - 1] = sum;
        }
    }

    ~RandomizedMessage() {
        delete[] data;
    }

    bool operator==(const RandomizedMessage &rhs) const {
        return size == rhs.size && memcmp(data, rhs.data, size) == 0;
    }

    bool valid() const {
        uint8_t sum = 0;
        for (size_t i = 0; i < size - 1; ++i) {
            sum += data[i];
        }
        return sum == data[size - 1];
    }
};

dunit("udp-broadcast-socket", "send/recv")
.workers(1)
.driver([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    RandomizedMessage m(SMALL_MESSAGE_SIZE, false);
    assert(s.recv(m.data, m.size) == m.size);

    assert(m.valid());

    dtest_notify();
})
.worker([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    RandomizedMessage m(SMALL_MESSAGE_SIZE);
    s.send(m.data, m.size);

    dtest_wait();
});

dunit("udp-broadcast-socket", "send/recv-many")
.workers(1)
.driver([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    RandomizedMessage m(SMALL_MESSAGE_SIZE, false);
    for (auto i = 0; i < MANY_MESSAGES; ++i) {
        assert(s.recv(m.data, m.size) == m.size);
        assert(m.valid());
    }

    dtest_notify();
})
.worker([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    for (auto i = 0; i < MANY_MESSAGES; ++i) {
        RandomizedMessage m(SMALL_MESSAGE_SIZE);
        s.send(m.data, m.size);
    }

    dtest_wait();
});

dunit("udp-broadcast-socket", "echo-small")
.workers(1)
.driver([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    RandomizedMessage m(SMALL_MESSAGE_SIZE);
    s.send(m.data, m.size);

    RandomizedMessage m2(SMALL_MESSAGE_SIZE, false);
    assert(s.recv(m2.data, m.size) == m.size);
    assert(m2 == m);

    dtest_notify();
})
.worker([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    RandomizedMessage m(SMALL_MESSAGE_SIZE, false);
    assert(s.recv(m.data, m.size) == m.size);
    assert(m.valid());

    s.send(m.data, m.size);

    dtest_wait();
});

dunit("udp-broadcast-socket", "echo-medium")
.workers(1)
.driver([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    RandomizedMessage m(MEDIUM_MESSAGE_SIZE);
    s.send(m.data, m.size);

    RandomizedMessage m2(MEDIUM_MESSAGE_SIZE, false);
    assert(s.recv(m2.data, m.size) == m.size);
    assert(m2 == m);

    dtest_notify();
})
.worker([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    RandomizedMessage m(MEDIUM_MESSAGE_SIZE, false);
    assert(s.recv(m.data, m.size) == m.size);
    assert(m.valid());

    s.send(m.data, m.size);

    dtest_wait();
});

dunit("udp-broadcast-socket", "echo-large")
.workers(1)
.driver([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    RandomizedMessage m(LARGE_MESSAGE_SIZE);
    s.send(m.data, m.size);

    RandomizedMessage m2(LARGE_MESSAGE_SIZE, false);
    assert(s.recv(m2.data, m.size) == m.size);
    assert(m2 == m);

    dtest_notify();
})
.worker([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    RandomizedMessage m(LARGE_MESSAGE_SIZE, false);
    assert(s.recv(m.data, m.size) == m.size);
    assert(m.valid());

    s.send(m.data, m.size);

    dtest_wait();
});

dunit("udp-broadcast-socket", "echo-many")
.workers(1)
.driver([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    for (auto i = 0; i < MANY_MESSAGES; ++i) {
        RandomizedMessage m(MEDIUM_MESSAGE_SIZE);
        s.send(m.data, m.size);

        RandomizedMessage m2(MEDIUM_MESSAGE_SIZE, false);
        assert(s.recv(m2.data, m.size) == m.size);
        assert(m2 == m);
    }

    dtest_notify();
})
.worker([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    RandomizedMessage m(MEDIUM_MESSAGE_SIZE, false);
    for (auto i = 0; i < MANY_MESSAGES; ++i) {
        assert(s.recv(m.data, m.size) == m.size);
        assert(m.valid());

        s.send(m.data, m.size);
    }

    dtest_wait();
});

dunit("udp-broadcast-socket", "echo-many-faulty")
.faultyNetwork(0.99, 1)
.workers(1)
.driver([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    for (auto i = 0; i < MANY_MESSAGES; ++i) {
        RandomizedMessage m(MEDIUM_MESSAGE_SIZE);
        s.send(m.data, m.size);

        RandomizedMessage m2(MEDIUM_MESSAGE_SIZE, false);
        assert(s.recv(m2.data, m.size) == m.size);
        assert(m2 == m);
    }

    dtest_notify();
})
.worker([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    RandomizedMessage m(MEDIUM_MESSAGE_SIZE, false);
    for (auto i = 0; i < MANY_MESSAGES; ++i) {
        assert(s.recv(m.data, m.size) == m.size);
        assert(m.valid());

        s.send(m.data, m.size);
    }

    dtest_wait();
});

dunit("udp-broadcast-socket", "flood")
// .enable()
.inProcess()
.timeout(20)
.workers(1)
.driver([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    RandomizedMessage m(LARGE_MESSAGE_SIZE);
    for (auto i = 0; i < MANY_MESSAGES; ++i) {
        s.send(m.data, m.size);
    }

    dtest_wait();
})
.worker([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    RandomizedMessage m(LARGE_MESSAGE_SIZE, false);
    for (auto i = 0; i < MANY_MESSAGES; ++i) {
        assert(s.recv(m.data, m.size) == m.size);
        assert(m.valid());
    }

    dtest_notify();
});

dunit("udp-broadcast-socket", "flood-faulty")
.timeout(20)
.faultyNetwork(0.99, 1)
.workers(1)
.driver([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    RandomizedMessage m(LARGE_MESSAGE_SIZE, false);
    for (auto i = 0; i < MANY_MESSAGES; ++i) {
        assert(s.recv(m.data, m.size) == m.size);
        assert(m.valid());
    }

    dtest_notify();
})
.worker([] {
    UDPBroadcastSocket s(0);
    dtest_send_msg(s.address());
    SocketAddress addr;
    dtest_recv_msg(addr);
    s.broadcastAddresses({ addr });

    RandomizedMessage m(LARGE_MESSAGE_SIZE);
    for (auto i = 0; i < MANY_MESSAGES; ++i) {
        s.send(m.data, m.size);
    }

    dtest_wait();
});
