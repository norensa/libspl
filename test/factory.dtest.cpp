#include <dtest.h>
#include <factory.h>

module("factory")
.dependsOn({
    "hash-map"
});

using namespace spl;

unit("factory", "custom-factory")
.ignoreMemoryLeak()         // registerFactory() allocates memory
.body([] {
    struct A {
        int x;
    };

    Factory::registerFactory(typeid(A), [] {
        auto a = new A();
        a->x = 33;
        return a;
    });

    auto a = Factory::createObject<A>();
    assert(a->x == 33);
    delete a;
});

unit("factory", "createObject(type_info)")
.ignoreMemoryLeak()         // registerFactory() allocates memory
.body([] {
    struct A {
        virtual ~A() = default;
        virtual char f() {
            return 'A';
        }
    };

    struct B : A {
        char f() {
            return 'B';
        }
    };

    struct C : A {
        char f() {
            return 'C';
        }
    };

    Factory::registerFactory(typeid(A), [] { return new A(); });
    Factory::registerFactory(typeid(B), [] { return new B(); });
    Factory::registerFactory(typeid(C), [] { return new C(); });

    A *a = Factory::createObject<A>();
    assert(a->f() == 'A');
    delete a;

    a = Factory::createObject<A>(typeid(B));
    assert(a->f() == 'B');
    delete a;

    a = Factory::createObject<A>(typeid(C));
    assert(a->f() == 'C');
    delete a;
});

unit("factory", "createObject(size_t)")
.ignoreMemoryLeak()         // registerFactory() allocates memory
.body([] {
    struct A {
        virtual ~A() = default;
        virtual char f() {
            return 'A';
        }
    };

    struct B : A {
        char f() {
            return 'B';
        }
    };

    struct C : A {
        char f() {
            return 'C';
        }
    };

    Factory::registerFactory(typeid(A), [] { return new A(); });
    Factory::registerFactory(typeid(B), [] { return new B(); });
    Factory::registerFactory(typeid(C), [] { return new C(); });

    A *a = Factory::createObject<A>(typeid(A).hash_code());
    assert(a->f() == 'A');
    delete a;

    a = Factory::createObject<A>(typeid(B).hash_code());
    assert(a->f() == 'B');
    delete a;

    a = Factory::createObject<A>(typeid(C).hash_code());
    assert(a->f() == 'C');
    delete a;
});

unit("factory", "static-default-factory")
.body([] {
    struct A : WithFactory<A> {
        virtual ~A() = default;
        virtual char f() {
            return 'A';
        }
    };

    A x;

    A *a = Factory::createObject<A>(typeid(A).hash_code());
    assert(a->f() == 'A');
    delete a;
});
