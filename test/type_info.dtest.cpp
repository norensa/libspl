/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <type_info.h>

using namespace spl;

unit("type-info", "default-constructor")
.body([] {
    auto t = TypeInfo();
});

unit("type-info", "constructor")
.body([] {
    auto t = TypeInfo(typeid(int));
});

unit("type-info", "name")
.body([] {
    auto t = TypeInfo(typeid(int));

    assert(strcmp(t.name(), typeid(int).name()) == 0);
});

unit("type-info", "hashCode")
.body([] {
    auto t = TypeInfo(typeid(int));

    assert(t.hashCode() == typeid(int).hash_code());
});

unit("type-info", "equality")
.body([] {
    auto t = TypeInfo(typeid(int));

    assert(t == typeid(int));
    assert(typeid(int) == t);
});
