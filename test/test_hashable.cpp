#include <hash.h>
#include <traits.h>

using namespace spl;

struct HashableObj : Hashable {
    long v;
    void *buf;

    HashableObj()
    :   v(-1),
        buf(nullptr)
    { }

    HashableObj(long v)
    :   v(v),
        buf(malloc(1))
    { }

    HashableObj(const HashableObj &r)
    :   v(r.v),
        buf(malloc(1))
    { }

    HashableObj(HashableObj &&r)
    :   v(r.v),
        buf(r.buf)
    {
        r.buf = nullptr;
    }

    ~HashableObj() {
        if (buf != nullptr) {
            free(buf);
            buf = nullptr;
        }
    }

    HashableObj & operator=(const HashableObj &r) {
        if (this != &r) {
            if (buf != nullptr) free(buf);
            v = r.v;
            buf = malloc(1);
        }
        return *this;
    }

    HashableObj & operator=(HashableObj &&r) {
        if (this != &r) {
            if (buf != nullptr) free(buf);
            v = r.v;
            buf = r.buf;
            r.buf = nullptr;
        }
        return *this;
    }

    size_t hash() const {
        return (size_t) v;
    }

    bool operator==(const HashableObj &rhs) const {
        return v == rhs.v;
    }
};
