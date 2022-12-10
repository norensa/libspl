/*
 * Copyright (c) 2021-2022 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <range.h>

using namespace spl;

unit("range", "insert")
.body([] {

    Range<int> r;

    r.insert({ 0, 2 });

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 0);
    assert(it->end == 2);
    ++it;

    assert(it == r.end());
});

unit("range", "insert-middle-disjoint")
.body([] {

    Range<int> r;

    r.insert({ 1, 2 });
    r.insert({ 5, 6 });
    r.insert({ 3, 4 });

    auto it = r.begin();

    assert(it->start == 1);
    assert(it->end == 2);
    ++it;

    assert(it != r.end());
    assert(it->start == 3);
    assert(it->end == 4);
    ++it;

    assert(it != r.end());
    assert(it->start == 5);
    assert(it->end == 6);
    ++it;

    assert(it == r.end());
});

unit("range", "insert-middle-join-previous")
.body([] {

    Range<int> r;

    r.insert({ 1, 2 });
    r.insert({ 5, 6 });
    r.insert({ 2, 4 });

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 4);
    ++it;

    assert(it != r.end());
    assert(it->start == 5);
    assert(it->end == 6);
    ++it;

    assert(it == r.end());
});

unit("range", "insert-middle-join-next")
.body([] {

    Range<int> r;

    r.insert({ 1, 2 });
    r.insert({ 5, 6 });
    r.insert({ 3, 5 });

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 2);
    ++it;

    assert(it != r.end());
    assert(it->start == 3);
    assert(it->end == 6);
    ++it;

    assert(it == r.end());
});

unit("range", "insert-middle-join-all")
.body([] {

    Range<int> r;

    r.insert({ 1, 2 });
    r.insert({ 5, 6 });
    r.insert({ 2, 5 });

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 6);
    ++it;

    assert(it == r.end());
});

unit("range", "insert-begin-disjoint")
.body([] {

    Range<int> r;

    r.insert({ 3, 4 });
    r.insert({ 1, 2 });

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 2);
    ++it;

    assert(it != r.end());
    assert(it->start == 3);
    assert(it->end == 4);
    ++it;

    assert(it == r.end());
});

unit("range", "insert-begin-join")
.body([] {

    Range<int> r;

    r.insert({ 3, 4 });
    r.insert({ 1, 3 });

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 4);
    ++it;

    assert(it == r.end());
});

unit("range", "insert-end-disjoint")
.body([] {

    Range<int> r;

    r.insert({ 1, 2 });
    r.insert({ 3, 4 });

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 2);
    ++it;

    assert(it != r.end());
    assert(it->start == 3);
    assert(it->end == 4);
    ++it;

    assert(it == r.end());
});

unit("range", "insert-end-join")
.body([] {

    Range<int> r;

    r.insert({ 1, 3 });
    r.insert({ 3, 4 });

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 4);
    ++it;

    assert(it == r.end());
});

unit("range", "insert-over-not-equal")
.body([] {

    Range<int> r;

    r.insert({ 1, 2 });
    r.insert({ 3, 8 });
    r.insert({ 4, 5 });

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 2);
    ++it;

    assert(it != r.end());
    assert(it->start == 3);
    assert(it->end == 8);
    ++it;

    assert(it == r.end());
});

unit("range", "insert-over-equal-1")
.body([] {

    Range<int> r;

    r.insert({ 1, 2 });
    r.insert({ 3, 8 });
    r.insert({ 3, 5 });

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 2);
    ++it;

    assert(it != r.end());
    assert(it->start == 3);
    assert(it->end == 8);
    ++it;

    assert(it == r.end());
});

unit("range", "insert-over-equal-2")
.body([] {

    Range<int> r;

    r.insert({ 1, 2 });
    r.insert({ 3, 8 });
    r.insert({ 3, 8 });

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 2);
    ++it;

    assert(it != r.end());
    assert(it->start == 3);
    assert(it->end == 8);
    ++it;

    assert(it == r.end());
});

unit("range", "contains-value")
.body([] {

    Range<int> r;

    r.insert({ 1, 2 });
    r.insert({ 3, 8 });
    r.insert({ 9, 12 });

    assert(! r.contains(0));

    assert(r.contains(1));
    assert(! r.contains(2));

    assert(r.contains(3));
    assert(r.contains(4));
    assert(r.contains(5));
    assert(r.contains(6));
    assert(r.contains(7));
    assert(! r.contains(8));

    assert(r.contains(9));
    assert(r.contains(10));
    assert(r.contains(11));
    assert(! r.contains(12));

    assert(! r.contains(13));
});

unit("range", "contains-interval")
.body([] {

    Range<int> r;

    r.insert({ 1, 2 });
    r.insert({ 3, 8 });
    r.insert({ 9, 12 });

    assert(r.contains({ 1, 2 }));
    assert(r.contains({ 3, 8 }));
    assert(r.contains({ 9, 12 }));
    assert(r.contains({ 4, 5 }));
    assert(r.contains({ 3, 5 }));
    assert(r.contains({ 5, 8 }));

    assert(! r.contains({ 0, 2 }));
    assert(! r.contains({ 5, 10 }));
    assert(! r.contains({ 8, 8 }));
    assert(! r.contains({ 8, 9 }));
});

unit("range", "union-1")
.body([] {

    Range<int> r1;
    r1.insert({ 1, 2 });

    Range<int> r2;
    r2.insert({ 3, 4 });

    auto r = r1 | r2;

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 2);
    ++it;

    assert(it != r.end());
    assert(it->start == 3);
    assert(it->end == 4);
    ++it;

    assert(it == r.end());
});

unit("range", "union-2")
.body([] {

    Range<int> r1;
    r1.insert({ 1, 2 });
    r1.insert({ 5, 6 });

    Range<int> r2;
    r2.insert({ 3, 4 });
    r2.insert({ 7, 8 });

    auto r = r1 | r2;

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 2);
    ++it;

    assert(it != r.end());
    assert(it->start == 3);
    assert(it->end == 4);
    ++it;

    assert(it != r.end());
    assert(it->start == 5);
    assert(it->end == 6);
    ++it;

    assert(it != r.end());
    assert(it->start == 7);
    assert(it->end == 8);
    ++it;

    assert(it == r.end());
});

unit("range", "union-3")
.body([] {

    Range<int> r1;
    r1.insert({ 1, 3 });
    r1.insert({ 5, 6 });

    Range<int> r2;
    r2.insert({ 3, 4 });
    r2.insert({ 7, 8 });

    auto r = r1 | r2;

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 4);
    ++it;

    assert(it != r.end());
    assert(it->start == 5);
    assert(it->end == 6);
    ++it;

    assert(it != r.end());
    assert(it->start == 7);
    assert(it->end == 8);
    ++it;

    assert(it == r.end());
});

unit("range", "union-4")
.body([] {

    Range<int> r1;
    r1.insert({ 1, 3 });
    r1.insert({ 5, 6 });
    r1.insert({ 6, 7 });

    Range<int> r2;
    r2.insert({ 1, 8 });

    auto r = r1 | r2;

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 8);
    ++it;

    assert(it == r.end());
});

unit("range", "union-5")
.body([] {

    Range<int> r1;
    r1.insert({ 1, 3 });
    r1.insert({ 5, 6 });
    r1.insert({ 6, 7 });

    Range<int> r2;
    r2.insert({ 1, 8 });

    auto r = r2 | r1;

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 8);
    ++it;

    assert(it == r.end());
});

unit("range", "union-6")
.body([] {

    Range<int> r1;
    r1.insert({ 1, 3 });
    r1.insert({ 5, 6 });
    r1.insert({ 7, 8 });

    Range<int> r2;
    r2.insert({ 1, 9 });

    auto r = r1 | r2;

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 9);
    ++it;

    assert(it == r.end());
});

unit("range", "difference-1")
.body([] {

    Range<int> r1;
    r1.insert({ 1, 5 });

    Range<int> r2;
    r2.insert({ 3, 7 });

    auto r = r1 - r2;

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 3);
    ++it;

    assert(it == r.end());
});

unit("range", "difference-2")
.body([] {

    Range<int> r1;
    r1.insert({ 3, 7 });

    Range<int> r2;
    r2.insert({ 1, 5 });

    auto r = r1 - r2;

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 5);
    assert(it->end == 7);
    ++it;

    assert(it == r.end());
});

unit("range", "difference-3")
.body([] {

    Range<int> r1;
    r1.insert({ 1, 5 });
    r1.insert({ 7, 9 });

    Range<int> r2;
    r2.insert({ 3, 8 });

    auto r = r1 - r2;

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 3);
    ++it;

    assert(it != r.end());
    assert(it->start == 8);
    assert(it->end == 9);
    ++it;

    assert(it == r.end());
});

unit("range", "difference-4")
.body([] {

    Range<int> r1;
    r1.insert({ 3, 8 });

    Range<int> r2;
    r2.insert({ 1, 5 });
    r2.insert({ 7, 9 });

    auto r = r1 - r2;

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 5);
    assert(it->end == 7);
    ++it;

    assert(it == r.end());
});

unit("range", "difference-5")
.body([] {

    Range<int> r1;
    r1.insert({ 1, 5 });

    Range<int> r2;
    r2.insert({ 1, 5 });

    auto r = r1 - r2;

    auto it = r.begin();

    assert(it == r.end());
});

unit("range", "difference-6")
.body([] {

    Range<int> r1;
    r1.insert({ 1, 5 });
    r1.insert({ 9, 12 });

    Range<int> r2;
    r2.insert({ 8, 12 });

    auto r = r1 - r2;

    auto it = r.begin();

    assert(it != r.end());
    assert(it->start == 1);
    assert(it->end == 5);
    ++it;

    assert(it == r.end());
});
