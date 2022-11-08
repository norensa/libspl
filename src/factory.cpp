/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <factory.h>
#include <hash_map.h>
#include <exception.h>

using namespace spl;

static HashMap<size_t, void *> & _repo() {
    static HashMap<size_t, void *> __repo;

    return __repo;
}

void Factory::_put(size_t hashCode, void *factory) {
    if (_repo().contains(hashCode)) {
        throw RuntimeError("Duplicate object hash codes detected");
    }
    _repo().put(hashCode, factory);
}

void * Factory::_get(size_t hashCode) {
    try {
        return _repo().get(hashCode);
    }
    catch (const ElementNotFoundError &) {
        throw InvalidArgument("No registered factory for this object type");
    }
}
