#include <factory.h>

using namespace spl;

HashMap<size_t, void *> & Factory::_factory() {
    static HashMap<size_t, void *> __repo;

    return __repo;
}
