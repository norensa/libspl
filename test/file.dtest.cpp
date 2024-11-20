/*
 * Copyright (c) 2021-2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <dtest.h>
#include <file.h>
#include "test_serializable.cpp"
#include <vector>
#include <thread>
#include <atomic>

module("path")
.dependsOn({
    "exception",
});

module("file")
.dependsOn({
    "path",
    "exception",
    "list",
    "base64",
});

module("file-serializer")
.dependsOn({
    "file",
    "random-access-serializer",
});

using namespace spl;

#define TEST_SIZE (10240)

unit("path", "realpath")
.body([] {
    Path p(".");
    Path real = p.realpath();

    assert(real.get()[0] == '/');
    assert(strlen(real.get()) > 1);
});

unit("path", "base")
.body([] {
    assert(Path("/abc/def").base() == "def");
    assert(Path("/abc/def/").base() == "def");
});

unit("path", "parent")
.body([] {
    assert(strcmp(Path("/abc/def").parent().get(), "/abc") == 0);
    assert(strcmp(Path("/abc/def/").parent().get(), "/abc") == 0);
});

unit("path", "append")
.body([] {
    assert(strcmp(Path("/abc/def").append("ghi").get(), "/abc/def/ghi") == 0);
    assert(strcmp(Path("/abc/def/ghi/").append("jkl").get(), "/abc/def/ghi/jkl") == 0);
    assert(strcmp(Path("/abc/def/").append("ghi/", "jkl").get(), "/abc/def/ghi/jkl") == 0);
});

unit("file", "uniquePath")
.body([] {
    auto p = File::uniquePath(".");

    assert(access(p.get(), F_OK) != 0);
});

unit("file", "exists")
.body([] {
    mkdir("./test-dir", File::DEFAULT_NEW_DIRECTORY_MODE);

    assert(File::exists("./test-dir"));

    remove("./test-dir");
});

unit("file", "mkdir")
.body([] {
    File::mkdir("./test-dir");

    assert(access("./test-dir", F_OK) == 0);

    remove("./test-dir");
});

unit("file", "mkdirs")
.body([] {
    File::mkdirs("./test-dir/a/b");

    assert(access("./test-dir", F_OK) == 0);
    assert(access("./test-dir/a", F_OK) == 0);
    assert(access("./test-dir/a/b", F_OK) == 0);

    remove("./test-dir/a/b");
    remove("./test-dir/a");
    remove("./test-dir");
});

unit("file", "rmdirs")
.body([] {
    File::mkdirs("./test-dir/a/b");

    assert(access("./test-dir", F_OK) == 0);
    assert(access("./test-dir/a", F_OK) == 0);
    assert(access("./test-dir/a/b", F_OK) == 0);

    File::rmdirs("./test-dir");

    assert(access("./test-dir", F_OK) != 0);
});

unit("file", "remove")
.body([] {
    mkdir("./test-dir", File::DEFAULT_NEW_DIRECTORY_MODE);

    assert(access("./test-dir", F_OK) == 0);

    File::remove("./test-dir");

    assert(access("./test-dir", F_OK) != 0);
});

unit("file", "rename")
.body([] {
    mkdir("./test-dir", File::DEFAULT_NEW_DIRECTORY_MODE);

    assert(access("./test-dir", F_OK) == 0);

    File::rename("./test-dir", "./renamed-test-dir");

    assert(access("./test-dir", F_OK) != 0);
    assert(access("./renamed-test-dir", F_OK) == 0);

    remove("./renamed-test-dir");
});

unit("file", "create")
.body([] {
    File f("./test-file");

    f.open(File::READ_WRITE | File::CREATE);
    f.close();

    assert(access("./test-file", F_OK) == 0);

    remove("./test-file");
});

unit("file", "write/read")
.body([] {
    File f("./test-file");

    f.open(File::READ_WRITE | File::CREATE);

    int *a = new int[TEST_SIZE];
    for (int i = 0; i < TEST_SIZE; ++i) {
        a[i] = dtest_random() * TEST_SIZE;
    }

    f.write(a, sizeof(int) * TEST_SIZE);
    f.close();

    int *b = new int[TEST_SIZE];
    f.read(b, sizeof(int) * TEST_SIZE);
    f.close();

    assert(memcmp(a, b, sizeof(int) * TEST_SIZE) == 0);

    delete[] a;
    delete[] b;

    remove("./test-file");
});

unit("file", "allocate")
.body([] {
    File f("./test-file");

    f.open(File::READ_WRITE | File::CREATE);
    assert(f.info().length() == 0);

    auto blk = f.info().blockSize();

    f.allocate(0, blk);
    assert(f.info().length() == blk);
    assert(f.info().numBlocks() == blk / 512);

    f.allocate(blk * 2, blk);
    assert(f.info().length() == 3 * blk);
    assert(f.info().numBlocks() == 2 * blk / 512);

    f.close();

    remove("./test-file");
});

unit("file", "deallocate")
.body([] {
    File f("./test-file");

    f.open(File::READ_WRITE | File::CREATE);
    assert(f.info().length() == 0);

    auto blk = f.info().blockSize();

    f.allocate(0, blk);
    assert(f.info().length() == blk);
    assert(f.info().numBlocks() == blk / 512);

    f.allocate(blk * 2, blk);
    assert(f.info().length() == 3 * blk);
    assert(f.info().numBlocks() == 2 * blk / 512);

    f.deallocate(0, blk);
    assert(f.info().length() == 3 * blk);
    assert(f.info().numBlocks() == blk / 512);

    f.close();

    remove("./test-file");
});

unit("file", "insert")
.body([] {
    File f("./test-file");

    f.open(File::READ_WRITE | File::CREATE);

    auto blk = f.info().blockSize();
    auto l = blk / sizeof(int) * 2;

    int *a = new int[l];
    for (int i = 0; i < (int) l; ++i) {
        a[i] = i;
    }

    f.write(a, blk * 2);
    f.close();
    assert(f.info().length() == 2 * blk);
    assert(f.info().numBlocks() == 2 * blk / 512);

    f.insert(blk, blk);
    f.write(blk, a, blk);
    f.close();
    assert(f.info().length() == 3 * blk);
    assert(f.info().numBlocks() == 3 * blk / 512);

    auto l2 = blk / sizeof(int) * 3;
    int *b = new int[l2];
    f.read(b, blk * 3);
    f.close();

    assert(memcmp(a, b, blk) == 0);
    assert(memcmp(a, (char *) b + blk, blk) == 0);
    assert(memcmp((char *) a + blk, (char *) b + 2 * blk, blk) == 0);
    assert(memcmp(a, b, blk * 2) != 0);

    delete[] a;
    delete[] b;

    remove("./test-file");
});

unit("file", "collapse")
.body([] {
    File f("./test-file");

    f.open(File::READ_WRITE | File::CREATE);

    auto blk = f.info().blockSize();
    auto l = blk / sizeof(int) * 2;

    int *a = new int[l];
    for (int i = 0; i < (int) l; ++i) {
        a[i] = i;
    }

    f.write(a, blk * 2);
    f.close();
    assert(f.info().length() == 2 * blk);
    assert(f.info().numBlocks() == 2 * blk / 512);

    f.collapse(0, blk);
    f.close();
    assert(f.info().length() == blk);
    assert(f.info().numBlocks() == blk / 512);

    auto l2 = blk / sizeof(int);
    int *b = new int[l2];
    f.read(b, blk);
    f.close();

    assert(memcmp((char *) a + blk, b, blk) == 0);

    delete[] a;
    delete[] b;

    remove("./test-file");
});

unit("file", "map")
.body([] {
    File f("./test-file");

    f.open(File::READ_WRITE | File::CREATE);

    int *a = new int[TEST_SIZE];
    for (int i = 0; i < TEST_SIZE; ++i) {
        a[i] = dtest_random() * i;
    }

    f.write(a, sizeof(int) * TEST_SIZE);
    f.close();

    auto m = f.map();
    f.close();

    assert(memcmp(a, m.ptr(), sizeof(int) * TEST_SIZE) == 0);

    delete[] a;

    remove("./test-file");
});

unit("file", "map-sync")
.body([] {
    File f("./test-file");

    f.open(File::READ_WRITE | File::CREATE);

    int *a = new int[TEST_SIZE];
    for (int i = 0; i < TEST_SIZE; ++i) {
        a[i] = dtest_random() * i;
    }

    f.write(a, sizeof(int) * TEST_SIZE);
    f.close();

    {
        auto m = f.map();
        f.close();
        assert(memcmp(a, m.ptr(), sizeof(int) * TEST_SIZE) == 0);
    }

    for (int i = 0; i < TEST_SIZE; ++i) {
        a[i] = dtest_random() * i;
    }

    {
        auto m = f.map(true);
        f.close();
        memcpy(m.ptr(), a, sizeof(int) * TEST_SIZE);
        m.sync(true);
    }

    {
        auto m = f.map();
        f.close();
        assert(memcmp(a, m.ptr(), sizeof(int) * TEST_SIZE) == 0);
    }

    delete[] a;

    remove("./test-file");
});

unit("file", "lock")
.body([] {
    File f("./test-file");
    f.open(File::READ_WRITE | File::CREATE);

    std::atomic_size_t count(0);
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.push_back(std::thread([&count] {
            File f("./test-file");
            for (int j = 0; j < 10000; ++j) {
                assert(f.lock());
                ++count;
                assert(count == 1);
                --count;
                f.unlock();
            }
        }));
    }

    for (auto & t : threads) t.join();

    f.close();
    remove("./test-file");
});

unit("file", "lock_test")
.body([] {
    File f("./test-file");
    f.open(File::READ_WRITE | File::CREATE);

    File f2("./test-file");

    assert(f2.lock_test());
    assert(f.lock());
    assert(! f2.lock_test());
    f.unlock();
    assert(f2.lock_test());

    f.close();
    f2.close();

    remove("./test-file");
});

unit("file-serializer", "primitive-types")
.body([] {
    File f("./test-file");

    f.open(File::READ_WRITE | File::CREATE);
    f.close();

    int x = 1;
    long y = 2;
    short z = 3;

    OutputFileSerializer out(f);
    out << x << y << z;
    out.flush();

    int x1;
    long y1;
    short z1;

    InputFileSerializer in(f);
    in >> x1 >> y1 >> z1;

    assert(x == x1);
    assert(y == y1);
    assert(z == z1);

    remove("./test-file");
});

unit("file-serializer", "serializable-type")
.body([] {
    File f("./test-file");

    f.open(File::READ_WRITE | File::CREATE);
    f.close();

    auto elem = RandomAccessSerializable();

    OutputFileSerializer out(f);
    out << elem;
    out.flush();
    assert(elem.serialized());

    auto elem1 = RandomAccessSerializable();

    InputFileSerializer in(f);
    in >> elem1;

    assert(elem1.deserialized());

    remove("./test-file");
});

unit("file-serializer", "large-serialization")
.body([] {
    File f("./test-file");

    f.open(File::READ_WRITE | File::CREATE);
    f.close();

    OutputFileSerializer out(f);
    for (auto i = 0; i < TEST_SIZE; ++i) {
        out << i;
    }
    out.flush();

    InputFileSerializer in(f);
    for (auto i = 0; i < TEST_SIZE; ++i) {
        int x;
        in >> x;
        assert(x == i);
    }

    remove("./test-file");
});

unit("file-serializer", "bulk-serialization")
.body([] {
    File f("./test-file");

    f.open(File::READ_WRITE | File::CREATE);
    f.close();

    int *a = new int[TEST_SIZE];
    for (auto i = 0; i < TEST_SIZE; ++i) {
        a[i] = dtest_random() * TEST_SIZE;
    }

    OutputFileSerializer out(f);
    out.put(a, TEST_SIZE * sizeof(int));
    out.flush();

    int *b = new int[TEST_SIZE];
    InputFileSerializer in(f);
    in.get(b, TEST_SIZE * sizeof(int));

    assert(memcmp(a, b, TEST_SIZE * sizeof(int)) == 0);

    delete[] a;
    delete[] b;

    remove("./test-file");
});
