/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <reference.h>

using namespace spl;

unit("reference", "empty")
.body([] {
    Reference<int> r;

    assert(r.get() == nullptr);
});

unit("reference", "create")
.body([] {
    Reference<int> r(new int());
});

unit("reference", "get")
.body([] {
    auto p = new int(3);
    Reference<int> r(p);

    assert(r.get() == p);
});

unit("reference", "conversion")
.body([] {
    auto p = new int(3);
    Reference<int> r(p);

    int x = r;
    assert(x == 3);
});
