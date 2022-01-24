/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <file.h>

using namespace spl;

#include <glob.h>
#include <cstring>
#include <base64.h>

Path File::uniquePath(const char *dir, const char *prefix) {
    using namespace std::chrono;

    Path p(dir);
    std::string pre(prefix);
    Path unique;

    do {
        uint64_t now = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
        size_t len;
        char *tmp = Base64::encode(&now, sizeof(now), len);
        char *unsafe = nullptr;
        while ((unsafe = strchr(tmp, '/')) != nullptr) *unsafe = '-';
        unique = p.append((pre + tmp).c_str());
        free(tmp);
    } while (exists(unique));

    return unique;
}

void File::mkdirs(const char *path) {
    char *tmp = strdup(path);
    size_t len = strlen(tmp);

    if(tmp[len - 1] == Path::SEPARATOR) {
        tmp[len - 1] = '\0';
    }

    bool noExist = false;
    for(char *p = tmp + 1; *p != '\0'; ++p) {
        if(*p == Path::SEPARATOR) {
            *p = '\0';
            if (noExist || ! exists(tmp)) {
                noExist = true;
                try { mkdir(tmp); }
                catch (Error &e) { free(tmp); throw e; }
            }
            *p = Path::SEPARATOR;
        }
    }
    if (! exists(tmp)) {
        try { mkdir(tmp); }
        catch (Error &e) { free(tmp); throw e; }
    }

    free(tmp);
}

void File::rmdirs(const Path &path) {
    PathInfo pi(path);

    if (pi.isDir()) {
        list(path.append("*")).foreach([] (const Path &path) {
            rmdirs(path);
        });
    }

    remove(path);
}

List<Path> File::list(const char *pattern) {
    List<Path> children;

    glob_t globbuf; globbuf.gl_offs = 0;
    if (glob(pattern, GLOB_DOOFFS, NULL, &globbuf) == 0) {
        for (size_t i = 0; i < globbuf.gl_pathc; ++i) {
            children.append(globbuf.gl_pathv[i]);
        }
    }
    globfree(&globbuf);

    return children;
}

size_t File::read(void *buf, size_t len) {
    if (_fd == -1) open();
    size_t readBytes = 0;
    while (len > 0) {
        ssize_t x = ::read(_fd, (uint8_t *) buf + readBytes, len);
        if (x == -1) throw ErrnoRuntimeError();
        if (x == 0) break;
        readBytes += x;
        len -= x;
    }
    return readBytes;
}

size_t File::read(size_t offset, void *buf, size_t len) {
    if (_fd == -1) open();
    size_t readBytes = 0;
    while (len > 0) {
        ssize_t x = ::pread(_fd, (uint8_t *) buf + readBytes, len, offset + readBytes);
        if (x == -1) throw ErrnoRuntimeError();
        if (x == 0) break;
        readBytes += x;
        len -= x;
    }
    return readBytes;
}

void File::write(const void *buf, size_t len) {
    if (_fd == -1) open();
    size_t writtenBytes = 0;
    while (len > 0) {
        ssize_t x = ::write(_fd, (uint8_t *) buf + writtenBytes, len);
        if (x == -1) throw ErrnoRuntimeError();
        writtenBytes += x;
        len -= x;
    }
    _info.clear();
}

void File::write(size_t offset, const void *buf, size_t len) {
    if (_fd == -1) open();
    size_t writtenBytes = 0;
    while (len > 0) {
        ssize_t x = ::pwrite(_fd, (uint8_t *) buf + writtenBytes, len, offset + writtenBytes);
        if (x == -1) throw ErrnoRuntimeError();
        writtenBytes += x;
        len -= x;
    }
    _info.clear();
}

File & File::allocate(size_t offset, size_t len) {
    if (_fd == -1) open();
    if (fallocate(_fd, 0, offset, len) != 0) {
        throw ErrnoRuntimeError();
    }
    _info.clear();
    return *this;
}

File & File::deallocate(size_t offset, size_t len) {
    if (_fd == -1) open();
    if (fallocate(_fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, offset, len) != 0) {
        throw ErrnoRuntimeError();
    }
    _info.clear();
    return *this;
}

File & File::insert(size_t offset, size_t len) {
    if (_fd == -1) open();
    if (fallocate(_fd, FALLOC_FL_INSERT_RANGE, offset, len) != 0) {
        throw ErrnoRuntimeError();
    }
    _info.clear();
    return *this;
}

File & File::collapse(size_t offset, size_t len) {
    if (_fd == -1) open();
    if (fallocate(_fd, FALLOC_FL_COLLAPSE_RANGE, offset, len) != 0) {
        throw ErrnoRuntimeError();
    }
    _info.clear();
    return *this;
}

MemoryMapping File::map(size_t offset, size_t len, bool writeable) {
    if (_fd == -1) open();

    int flags = MAP_NONBLOCK | MAP_NORESERVE;
    if (writeable) flags |= MAP_SHARED;

    int prot = PROT_READ;
    if (writeable) prot |= PROT_WRITE;

    void *ptr = mmap(
        nullptr,
        len,
        prot,
        flags,
        _fd,
        offset
    );

    if (ptr == MAP_FAILED) {
        throw ErrnoRuntimeError();
    }

    return MemoryMapping(ptr, len);
}
