/*
 * Copyright (c) 2021 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <exception.h>
#include <cstdlib>
#include <stdint.h>
#include <limits.h>
#include <string>
#include <libgen.h>
#include <sys/stat.h>
#include <chrono>
#include <unistd.h>
#include <cstdio>
#include <fcntl.h>
#include <list.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <serialization.h>

namespace spl {

/**
 * @brief Represents the path of an object in the filesystem.
 */
class Path {

private:

    char *_path = nullptr;

    static size_t _appendCount(const char *p) {
        return strlen(p);
    }

    template <typename ...C>
    static size_t _appendCount(const char *p1, C ...p2) {
        return strlen(p1) + 1 + _appendCount(p2...);
    }

    static void _append(char *&ptr, const char *child) {
        if (*(ptr - 1) != SEPARATOR) *ptr++ = SEPARATOR;
        size_t l = strlen(child);
        memcpy(ptr, child, l);
        ptr += l;
    }

    template <typename ...C>
    static void _append(char *&ptr, const char *child, C ...c) {
        _append(ptr, child);
        _append(ptr, c...);
    }

public:

    static constexpr char SEPARATOR = '/';

    /**
     * @brief Construct a new uninitialized Path object.
     */
    Path()
    :   _path(nullptr)
    { }

    /**
     * @brief Construct a new Path object.
     * 
     * @param path The desired path to hold.
     */
    Path(const char *path)
    :   _path(strdup(path))
    { }

    /**
     * @brief Construct a new Path object.
     * 
     * @param path The desired path to hold.
     */
    Path(const std::string &path)
    :   Path(path.c_str())
    { }

    Path(const Path &rhs)
    :   _path(rhs._path == nullptr ? nullptr : strdup(rhs._path))
    { }

    Path(Path &&rhs)
    :   _path(rhs._path)
    {
        rhs._path = nullptr;
    }

    ~Path() {
        if (_path != nullptr) free(_path);
    }

    Path & operator=(const Path& rhs) {
        if (this != &rhs) {
            if (_path != nullptr) free(_path);
            _path = rhs._path == nullptr ? nullptr : strdup(rhs._path);
        }
        return *this;
    }

    Path & operator=(Path &&rhs) {
        if (this != &rhs) {
            if (_path != nullptr) free(_path);
            _path = rhs._path;
            rhs._path = nullptr;
        }
        return *this;
    }

    bool operator==(const Path &rhs) const {
        return _path != nullptr
            && rhs._path != nullptr
            && strcmp(_path, rhs._path) == 0;
    }

    bool operator!=(const Path &rhs) const {
        return ! operator==(rhs);
    }

    /**
     * @return A pointer to a C-style string that holds the path.
     */
    const char * get() const {
        return _path;
    }

    /**
     * @brief Fetches the real path to return a fully canonicalized path without
     * symbolic links.
     * 
     * @return A new Path object that has the new real path.
     */
    Path realpath() const {
        Path p;
        p._path = ::realpath(_path, nullptr);
        if (p._path == nullptr) {
            throw ErrnoRuntimeError();
        }
        return p;
    }

    /**
     * @brief Appends a child to this path object.
     * 
     * @return A new Path object with the new composed path.
     */
    Path append(const char *child) const {
        Path p;
        size_t l1 = strlen(_path);
        size_t l2 = strlen(child);
        p._path = (char *) malloc(l1 + l2 + 2);
        char *ptr = p._path;

        memcpy(ptr, _path, l1);
        ptr += l1;

        _append(ptr, child);

        *ptr = '\0';

        return p;
    }

    /**
     * @brief Appends multiple children to this path object.
     * 
     * @return A new Path object with the new composed path.
     */
    template <typename ...Children>
    Path append(const char *child, Children... children) const {
        Path p;
        p._path = (char *) malloc(_appendCount(_path, child, children...));

        char *ptr = p._path;
        size_t l1 = strlen(_path);
        memcpy(ptr, _path, l1);
        ptr += l1;

        _append(ptr, child, children...);

        *ptr = '\0';

        return p;
    }

    /**
     * @brief Finds the parent of this path object.
     * 
     * @return A new Path object that has the paren't path.
     */
    Path parent() const {
        Path p(*this);
        dirname(p._path);
        return p;
    }

    /**
     * @brief Finds the base name of this path object.
     * 
     * @return The base name.
     */
    std::string base() const {
        char *p = strdup(_path);
        std::string b(basename(p));
        free(p);
        return b;
    }
};

/**
 * @brief Retrieves information on a file, directory, and other named objects in
 * the filesystem.
 */
class PathInfo {

private:

    Path _path;
    struct stat *_stat;

    void _fetchStat() const {
        if (_stat == nullptr) {
            const_cast<PathInfo *>(this)->_stat = new struct stat();
            if (stat(_path.get(), _stat) != 0) {
                throw ErrnoRuntimeError();
            }
        }
    }

public:

    /**
     * @brief Construct a new uninitialized PathInfo object.
     */
    PathInfo()
    :   _stat(nullptr)
    { }

    /**
     * @brief Construct a new PathInfo object.
     * 
     * @param path Path of some filesystem object.
     */
    PathInfo(const Path &path)
    :   _path(path),
        _stat(nullptr)
    { }

    /**
     * @brief Construct a new PathInfo object.
     * 
     * @param path Path of some filesystem object.
     */
    PathInfo(Path &&path)
    :   _path(std::move(path)),
        _stat(nullptr)
    { }

    PathInfo(const PathInfo &rhs)
    :   _path(rhs._path),
        _stat(rhs._stat == nullptr ? nullptr : new struct stat(*rhs._stat))
    { }

    PathInfo(PathInfo &&rhs)
    :   _path(std::move(rhs._path)),
        _stat(std::move(rhs._stat))
    {
        rhs._stat = nullptr;
    }

    ~PathInfo() {
        if (_stat != nullptr) delete _stat;
    }

    PathInfo & operator=(const PathInfo &rhs) {
        if (this != &rhs) {
            if (_stat != nullptr) delete _stat;

            _path = rhs._path;
            _stat = rhs._stat == nullptr ? nullptr : new struct stat(*rhs._stat);
        }
        return *this;
    }

    PathInfo & operator=(PathInfo &&rhs) {
        if (this != &rhs) {
            if (_stat != nullptr) delete _stat;

            _path = std::move(rhs._path);
            _stat = rhs._stat;
            rhs._stat = nullptr;
        }
        return *this;
    }

    /**
     * @brief Forces this PathInfo object to retrieve any missing information.
     * 
     * @return A reference to this object for chaining.
     */
    PathInfo & fetch() {
        _fetchStat();
        return *this;
    }

    /**
     * @brief Clears all internally-cached information.
     * 
     * @return A reference to this object for chaining.
     */
    PathInfo & clear() {
        if (_stat != nullptr) {
            delete _stat;
            _stat = nullptr;
        }
        return *this;
    }

    /**
     * @return A const reference to the internal Path object.
     */
    const Path & path() const {
        return _path;
    }

    /**
     * @return The reported length of the file (if it is a regular file or a
     * symbolic link) in bytes. This will include any potential holes in the
     * file. For symbolic links, the length of the file is that of the pathname
     * it contains (without a terminating null byte).
     */
    off_t length() const {
        _fetchStat();
        return _stat->st_size;
    }

    /**
     * @return True if the object is a file, false otherwise.
     */
    bool isFile() const {
        _fetchStat();
        return S_ISREG(_stat->st_mode);
    }

    /**
     * @return True if the object is a directory, false otherwise.
     */
    bool isDir() const {
        _fetchStat();
        return S_ISDIR(_stat->st_mode);
    }

    /**
     * @return True if the object is a character device, false otherwise.
     */
    bool isCharacterDevice() const {
        _fetchStat();
        return S_ISCHR(_stat->st_mode);
    }

    /**
     * @return True if the object is a block device, false otherwise.
     */
    bool isBlockDevice() const {
        _fetchStat();
        return S_ISBLK(_stat->st_mode);
    }

    /**
     * @return True if the object is a named pipe, false otherwise.
     */
    bool isPipe() const {
        _fetchStat();
        return S_ISFIFO(_stat->st_mode);
    }

    /**
     * @return The "preferred" block size for efficient filesystem I/O.
     */
    blksize_t blockSize() const {
        _fetchStat();
        return _stat->st_blksize;
    }

    /**
     * @return The number of blocks allocated to the file, in 512-byte units. 
     * (This may be smaller than size/512 when the file has holes.)
     */
    blkcnt_t numBlocks() const {
        _fetchStat();
        return _stat->st_blocks;
    }

    /**
     * @return The user ID of the owner of the file.
     */
    uid_t uid() const {
        _fetchStat();
        return _stat->st_uid;
    }

    /**
     * @return The ID of the group owner of the file.
     */
    gid_t gid() const {
        _fetchStat();
        return _stat->st_gid;
    }

    /**
     * @return The time of the last access of file data.
     */
    auto accessTime() const {
        _fetchStat();
        return std::chrono::high_resolution_clock::time_point(
            std::chrono::nanoseconds(
                _stat->st_atim.tv_sec * 1000000000UL
                + _stat->st_atim.tv_nsec
            )
        );
    }

    /**
     * @return The time of last modification of file data.
     */
    auto modifyTime() const {
        _fetchStat();
        return std::chrono::high_resolution_clock::time_point(
            std::chrono::nanoseconds(
                _stat->st_mtim.tv_sec * 1000000000UL
                + _stat->st_mtim.tv_nsec
            )
        );
    }

    /**
     * @return The file's last status change timestamp (time of last change to
     * the inode).
     */
    auto statusChangeTime() const {
        _fetchStat();
        return std::chrono::high_resolution_clock::time_point(
            std::chrono::nanoseconds(
                _stat->st_ctim.tv_sec * 1000000000UL
                + _stat->st_ctim.tv_nsec
            )
        );
    }
};

/**
 * @brief An exception thrown to indicate that the requested operation requires
 * the file to be opened and it was found to be otherwise.
 */
class FileNotOpened
:   public Error
{
public:

    /**
     * @brief Construct a new FileNotOpened object.
     */
    FileNotOpened()
    :   Error("File is not opened")
    { }
};

/**
 * @brief Memory mapped buffer.
 */
class MemoryMapping {

    friend class File;

private:

    void *_mem = nullptr;
    size_t _size = 0;

    void _unmap() {
        if (_mem != nullptr) {
            munmap(_mem, _size);
        }
    }

    /**
     * @brief Construct a new MemoryMapping object.
     * 
     * @param mem Pointer to the mapped memory.
     * @param size Size of the memory mapping.
     */
    MemoryMapping(void *mem, size_t size)
    :   _mem(mem),
        _size(size)
    { }

public:

    MemoryMapping() = default;

    MemoryMapping(const MemoryMapping &) = delete;

    MemoryMapping(MemoryMapping &&rhs)
    :   _mem(rhs._mem),
        _size(rhs._size)
    {
        rhs._mem = nullptr;
        rhs._size = 0;
    }

    ~MemoryMapping() {
        _unmap();
    }

    MemoryMapping & operator=(const MemoryMapping &) = delete;

    MemoryMapping & operator=(MemoryMapping &&rhs) {
        if (this != &rhs) {
            _unmap();

            _mem = rhs._mem;
            _size = rhs._size;

            rhs._mem = nullptr;
            rhs._size = 0;
        }
        return *this;
    }

    /**
     * @return Pointer to the mapped memory.
     */
    void * ptr() {
        return _mem;
    }

    /**
     * @return Const pointer to the mapped memory.
     */
    const void * ptr() const {
        return _mem;
    }

    /**
     * @return The size of the mapped memory.
     */
    size_t size() const {
        return _size;
    }

    /**
     * @brief Synchronizes the changes done to the memory mapping with the
     * underlying storage.
     * 
     * @param block Indicates whether to block until the synchronization is
     * done. Default = false.
     */
    void sync(bool block = false) {
        int flags = block ? MS_SYNC : MS_ASYNC;
        if (msync(_mem, _size, flags) != 0) throw ErrnoRuntimeError();
    }

    /**
     * @brief Synchronizes the changes done to the memory mapping with the
     * underlying storage and invalidates all other mappings of the same region.
     * 
     * @param block Indicates whether to block until the synchronization is
     * done. Default = false.
     */
    void sync_invalidate(bool block = false) {
        int flags = MS_INVALIDATE;
        flags |= block ? MS_SYNC : MS_ASYNC;
        if (msync(_mem, _size, flags) != 0) throw ErrnoRuntimeError();
    }
};

/**
 * @brief A class for managing files, directories, and other named objects in
 * the filesystem.
 */
class File {

private:

    static const char * _getName(const char *name) {
        return name;
    }

    static const char * _getName(const std::string &name) {
        return name.c_str();
    }

    static const char * _getPath(const char *path) {
        return path;
    }

    static const char * _getPath(const std::string &path) {
        return path.c_str();
    }

    static const char * _getPath(const Path &path) {
        return path.get();
    }

public:

    /**
     * @brief The default directory creation mode. This is equivalent to 0755.
     */
    static constexpr mode_t DEFAULT_NEW_DIRECTORY_MODE = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

    /**
     * @brief The default file creation mode. This is equivalent to 0664.
     */
    static constexpr mode_t DEFAULT_NEW_FILE_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

    /**
     * @brief The default open flags. This opens the file in read/write mode.
     * 
     */
    static constexpr int DEFAULT_OPEN_FLAGS = O_RDWR;

    /**
     * @brief A flag to open the file in append mode, moving the file position
     * to the end of the file immediately after open.
     */
    static constexpr int APPEND = O_APPEND;

    /**
     * @brief A flag to open the file in read-only mode.
     */
    static constexpr int READ_ONLY = O_RDONLY;

    /**
     * @brief A flag to open the file in write-only mode.
     */
    static constexpr int WRITE_ONLY = O_WRONLY;

    /**
     * @brief A flag to open the file in read/write mode.
     */
    static constexpr int READ_WRITE = O_RDWR;

    /**
     * @brief An open flag to indicate creation of a new file.
     */
    static constexpr int CREATE = O_CREAT;

    /**
     * @brief An open flag to truncate the file if it already exists.
     */
    static constexpr int TRUNCATE = O_TRUNC;

    /**
     * @brief An open flag to disable the use of filesystem buffers for this
     * file.
     */
    static constexpr int DIRECT = O_DIRECT;

    /**
     * @brief An open flag to indicate the creation of an unnamed temporary
     * file. The path of the File object determines the location of the created
     * unnamed file. The file is automatically removed by the system.
     */
    static constexpr int UNNAMED_TEMP = O_TMPFILE;

    /**
     * @brief Generates a unique path to a non-existing file.
     * 
     * @param[in] dir Directory under which a unique path will be generated.
     * @param[in] prefix An optional prefix to prepend to the uniquely generated
     * name.
     * @return A Path object containing the generated path.
     */
    static Path uniquePath(const char *dir, const char *prefix = "");

    /**
     * @brief Generates a unique path to a non-existing file.
     * 
     * @param[in] dir Directory under which a unique path will be generated.
     * @param[in] prefix An optional prefix to prepend to the uniquely generated
     * name.
     * @return A Path object containing the generated path.
     */
    template <typename P1, typename P2>
    static Path uniquePath(const P1 &dir, const P2 &prefix) {
        return uniquePath(_getPath(dir), _getName(prefix));
    }

    /**
     * @brief Generates a unique path to a non-existing file.
     * 
     * @param[in] dir Directory under which a unique path will be generated.
     * @param[in] prefix An optional prefix to prepend to the uniquely generated
     * name.
     * @return A File object containing the generated path.
     */
    template <typename P1, typename P2>
    static File uniqueFile(const P1 &dir, const P2 &prefix) {
        return File(uniquePath(_getPath(dir), _getName(prefix)));
    }

    /**
     * @brief Tests for the existence of some path.
     * 
     * @param[in] path Path of the file or directory to test.
     * @return True if the file exists, false otherwise.
     */
    static bool exists(const char *path) {
        return access(path, F_OK) == 0;
    }

    /**
     * @brief Tests for the existence of some path.
     * 
     * @param[in] path Path of the file or directory to test.
     * @return True if the file exists, false otherwise.
     */
    template <typename P>
    static bool exists(const P &path) {
        return exists(_getPath(path));
    }

    /**
     * @brief Creates a directory.
     * 
     * @param[in] path The path of the directory to create.
     */
    static void mkdir(const char *path) {
        if (::mkdir(path, DEFAULT_NEW_DIRECTORY_MODE) != 0) {
            throw ErrnoRuntimeError();
        }
    }

    /**
     * @brief Creates a directory.
     * 
     * @param[in] path The path of the directory to create.
     */
    template <typename P>
    static void mkdir(const P &path) {
        mkdir(_getPath(path));
    }

    /**
     * @brief Creates a directory and any missing parent directories.
     * 
     * @param[in] path The path of the directory to create.
     */
    static void mkdirs(const char *path);

    /**
     * @brief Creates a directory and any missing parent directories.
     * 
     * @param[in] path The path of the directory to create.
     */
    template <typename P>
    static void mkdirs(const P &path) {
        mkdirs(_getPath(path));
    }

    /**
     * @brief Removes a file or directory.
     * 
     * @param[in] path The path of the file or directory to remove.
     */
    static void remove(const char *path) {
        if (::remove(path) != 0) {
            throw ErrnoRuntimeError();
        }
    }

    /**
     * @brief Removes a file or directory.
     * 
     * @param[in] path The path of the file or directory to remove.
     */
    template <typename P>
    static void remove(const P &path) {
        remove(_getPath(path));
    }

    /**
     * @brief Removes a directory and all of its contents.
     * 
     * @param[in] path The path of the directory to remove.
     */
    static void rmdirs(const Path &path);

    /**
     * @brief Renames and/or moves a file.
     * 
     * @param[in] oldPath The path of the file to rename and/or move.
     * @param[in] newPath The new path, having changed name and/or location.
     */
    static void rename(const char *oldPath, const char *newPath) {
        if (::rename(oldPath, newPath) != 0) {
            throw ErrnoRuntimeError();
        }
    }

    /**
     * @brief Renames and/or moves a file.
     * 
     * @param[in] oldPath The path of the file to rename and/or move.
     * @param[in] newPath The new path, having changed name and/or location.
     */
    template <typename P1, typename P2>
    static void rename(const P1 &oldPath, const P2 &newPath) {
        rename(_getPath(oldPath), _getPath(newPath));
    }

    /**
     * @brief Lists the files and directories matching a pattern.
     * 
     * @param pattern The pattern to search for.
     * @return A list of the files and directories found.
     */
    static List<Path> list(const char *pattern);

    /**
     * @brief Lists the files and directories matching a pattern.
     * 
     * @param pattern The pattern to search for.
     * @return A list of the files and directories found.
     */
    template <typename P>
    static List<Path> list(const P &pattern) {
        return list(_getPath(pattern));
    }

    /**
     * @brief Changes the current working directory.
     * 
     * @param path The new working directory.
     */
    static void chdir(const char *path) {
        if (::chdir(path) != 0) {
            throw ErrnoRuntimeError();
        }
    }

    /**
     * @brief Changes the current working directory.
     * 
     * @param path The new working directory.
     */
    template <typename P>
    static void chdir(const P &path) {
        chdir(_getPath(path));
    }

private:

    PathInfo _info;
    int _fd = -1;

    void _invalidate() {
        _fd = -1;
    }

    void _dispose() {
        close();
    }

    void _copy(const File &rhs) {
        _info = rhs._info;
        _fd = dup(rhs._fd);
    }

    void _move(File &rhs) {
        _info = std::move(rhs._info);
        _fd = rhs._fd;
    }

public:

    /**
     * @brief Construct a new File object.
     * 
     * @param path The path of the desired filesystem object.
     */
    File(const Path &path)
    :   _info(path),
        _fd(-1)
    { }

    /**
     * @brief Construct a new File object.
     * 
     * @param path The path of the desired filesystem object.
     */
    File(Path &&path)
    :   _info(std::move(path)),
        _fd(-1)
    { }

    File(const File &rhs) {
        _copy(rhs);
    }

    File(File &&rhs) {
        _move(rhs);
        rhs._invalidate();
    }

    ~File() {
        _dispose();
    }

    File & operator=(const File &rhs) {
        if (this != &rhs) {
            _dispose();
            _copy(rhs);
        }
        return *this;
    }

    File & operator=(File &&rhs) {
        if (this != &rhs) {
            _dispose();
            _move(rhs);
            rhs._invalidate();
        }
        return *this;
    }

    /**
     * @brief Closes the underlying file descriptor associated with this file.
     * 
     * @return A reference to this object for chaining.
     */
    File & close() {
        if (_fd != -1) {
            ::close(_fd);
            _fd = -1;
        }
        return *this;
    }

    /**
     * @return A const reference to the PathInfo object associated with this
     * file.
     */
    const PathInfo & info() const {
        return _info;
    }

    /**
     * @return True if the file exists, false otherwise.
     */
    bool exists() const {
        return exists(_info.path());
    }

    /**
     * @brief Creates the directory specified by this object.
     * 
     * @return A reference to this object for chaining.
     */
    File & mkdir() {
        mkdir(_info.path());
        return *this;
    }

    /**
     * @brief Creates the directory specified by this object, and any missing
     * parent directories.
     * 
     * @return A reference to this object for chaining.
     */
    File & mkdirs() {
        mkdirs(_info.path());
        return *this;
    }

    /**
     * @brief Removes the directory specified by this object and all of its
     * contents.
     * 
     * @return A reference to this object for chaining.
     */
    File & rmdirs() {
        rmdirs(_info.path());
        return *this;
    }

    /**
     * @brief Removes the file or directory specified by this object.
     * 
     * @return A reference to this object for chaining.
     */
    File & remove() {
        close();
        remove(_info.path());
        _info.clear();
        return *this;
    }

    /**
     * @brief Renames and/or moves the file specified by this object.
     * 
     * @param[in] newPath The new path, having changed name and/or location.
     * @return A reference to this object for chaining.
     */
    template <typename P>
    File & rename(const P &newPath) {
        close();
        rename(_info.path(), newPath);
        _info = PathInfo(newPath);
        return *this;
    }

    /**
     * @brief Changes the current working directory to that specified by this
     * object.
     * 
     * @return A reference to this object for chaining.
     */
    File & chdir() {
        chdir(_info.path());
        return *this;
    }

    /**
     * @brief Opens the file specified by this object.
     * 
     * @param[in] flags The open flags to pass to open(2). The default open mode
     * opens this file in read/write mode. See the manual pages for more
     * details.
     * @param[in] mode The creation mode to pass to open(2) if the file is to be
     * created as specified by the `flags` argument. The default creation mode
     * is 0755. See the manual pages for more details.
     * @return A reference to this object for chaining.
     */
    File & open(int flags = DEFAULT_OPEN_FLAGS, mode_t mode = DEFAULT_NEW_FILE_MODE)  {
        if (_fd == -1) {
            _fd = ::open(_info.path().get(), flags, mode);
            if (_fd == -1) {
                throw ErrnoRuntimeError();
            }
        }
        return *this;
    }

    /**
     * @brief Reads a block of data starting from the current file position.
     * 
     * @param[in] buf Pointer to a region of memory.
     * @param[in] len The number of bytes to read.
     * @return The number of bytes actually read. If 0 is returned, this means
     * EOF has been reached.
     */
    size_t read(void *buf, size_t len);

    /**
     * @brief Reads a block of data starting from the specified offset. The
     * internal file position is unaffected by this function.
     * 
     * @param[in] offset The offset (from the beginning of the file) to start
     * the read operation.
     * @param[in] buf Pointer to a region of memory.
     * @param[in] len The number of bytes to read.
     * @return The number of bytes actually read. If 0 is returned, this means
     * EOF has been reached.
     */
    size_t read(size_t offset, void *buf, size_t len);


    /**
     * @brief Writes a block of data starting at the current file position.
     * 
     * @param[in] buf Pointer to the data block.
     * @param[in] len The number of bytes to write.
     * @return The number of bytes actually written.
     */
    void write(const void *buf, size_t len);

    /**
     * @brief Writes a block of data starting at the specified offset. The
     * internal file position is unaffected by this function.
     * 
     * @param[in] offset The offset (from the beginning of the file) to start
     * the write operation.
     * @param[in] buf Pointer to the data block.
     * @param[in] len The number of bytes to write.
     * @return The number of bytes actually written.
     */
    void write(size_t offset, const void *buf, size_t len);

    /**
     * @return The current file position.
     */
    size_t pos() {
        if (_fd == -1) throw FileNotOpened();
        return lseek(_fd, 0, SEEK_CUR);
    }

    /**
     * @brief Sets the current file position.
     * 
     * @param[in] newPos The new file position.
     * @return A reference to this object for chaining.
     */
    File & pos(size_t newPos) {
        if (_fd == -1) throw FileNotOpened();
        lseek(_fd, newPos, SEEK_SET);
    }

    /**
     * @brief Displaces the current file position by the indicated number of
     * bytes.
     * 
     * @param[in] displacement The displacement in bytes for the file position.
     * This can be a positive or a negative value.
     * @return A reference to this object for chaining.
     */
    size_t movePos(ssize_t displacement) {
        if (_fd == -1) throw FileNotOpened();
        return lseek(_fd, displacement, SEEK_CUR);
    }

    /**
     * @brief Allocates disk space within a file.
     * 
     * @param offset Offset of the allocated area.
     * @param len Length of the allocated area.
     * @return A reference to this object for chaining.
     */
    File & allocate(size_t offset, size_t len);

    /**
     * @brief Deallocated disk space within a file, leaving a hole.
     * 
     * @param offset Offset of the area to deallocate.
     * @param len Length of the area to deallocate.
     * @return A reference to this object for chaining.
     */
    File & deallocate(size_t offset, size_t len);

    /**
     * @brief Inserts newly allocated disk space into a file.
     * 
     * @param offset Offset of the allocated area.
     * @param len Length of the allocated area.
     * @return A reference to this object for chaining.
     */
    File & insert(size_t offset, size_t len);

    /**
     * @brief Deallocates disk space within a file, collapsing the deallocated
     * region.
     * 
     * @param offset Offset of the area to deallocate.
     * @param len Length of the area to deallocate.
     * @return A reference to this object for chaining.
     */
    File & collapse(size_t offset, size_t len);

    /**
     * @brief Creates a memory mapping of a region in the file.
     * 
     * @param offset Offset of the mapped region.
     * @param len Length of the mapped region.
     * @param writeable Determines whether the memory mapping should be
     * writeable. If the writable=false, then the memory mapping will be
     * write-protected.
     * @return The memory mapped buffer.
     */
    MemoryMapping map(size_t offset, size_t len, bool writeable = true);

    /**
     * @brief Creates a memory mapping of the entire file.
     * 
     * @param writeable Determines whether the memory mapping should be
     * writeable. If the writable=false, then the memory mapping will be
     * write-protected.
     * @return The memory mapped buffer.
     */
    MemoryMapping map(bool writeable = true) {
        return map(0, _info.clear().length(), writeable);
    }
};

/**
 * @brief An output random-access serializer using File objects.
 */
class OutputFileSerializer
:   public OutputRandomAccessSerializer
{
private:

    File _f;
    size_t _maxLen;

protected:

    void _writeAt(size_t position, const void *data, size_t len) override {
        _f.write(position, data, len);
    }

    size_t _getLength() const override {
        return _maxLen;
    }

public:

    /**
     * @brief Construct a new OutputFileSerializer object.
     * 
     * @param f A file object to use for serialization.
     * @param maxLength The maximum allowed file length (default = SIZE_MAX).
     */
    OutputFileSerializer(const File &f, size_t maxLength = SIZE_MAX)
    :   _f(f),
        _maxLen(maxLength)
    { }

    /**
     * @brief Construct a new OutputFileSerializer object.
     * 
     * @param f A file object to use for serialization.
     * @param maxLength The maximum allowed file length (default = SIZE_MAX).
     */
    OutputFileSerializer(File &&f, size_t maxLength = SIZE_MAX)
    :   _f(std::move(f)),
        _maxLen(maxLength)
    { }

    /**
     * @brief Construct a new OutputFileSerializer object.
     * 
     * @param f A file object to use for serialization.
     * @param maxLength The maximum allowed file length (default = SIZE_MAX).
     * @param bufferSize Size of the internal serializer buffer.
     */
    OutputFileSerializer(const File &f, size_t maxLength, size_t bufferSize)
    :   OutputRandomAccessSerializer(bufferSize),
        _f(f),
        _maxLen(maxLength)
    { }

    /**
     * @brief Construct a new OutputFileSerializer object.
     * 
     * @param f A file object to use for serialization.
     * @param maxLength The maximum allowed file length (default = SIZE_MAX).
     * @param bufferSize Size of the internal serializer buffer.
     */
    OutputFileSerializer(File &&f, size_t maxLength, size_t bufferSize)
    :   OutputRandomAccessSerializer(bufferSize),
        _f(std::move(f)),
        _maxLen(maxLength)
    { }
};

/**
 * @brief An input random-access serializer using File objects.
 * 
 */
class InputFileSerializer
:   public InputRandomAccessSerializer
{
private:

    File _f;

protected:

    void _readAt(size_t position, void *data, size_t len) override {
        if (_f.read(position, data, len) != len) {
            throw RuntimeError("Failed to read the required bytes from file");
        }
    }

    size_t _getLength() const override {
        return _f.info().length();
    }

public:

    /**
     * @brief Construct a new InputFileSerializer object.
     * 
     * @param f A file object to use for serialization.
     */
    InputFileSerializer(const File &f)
    :   _f(f)
    { }

    /**
     * @brief Construct a new InputFileSerializer object.
     * 
     * @param f A file object to use for serialization.
     */
    InputFileSerializer(File &&f)
    :   _f(std::move(f))
    { }

    /**
     * @brief Construct a new InputFileSerializer object.
     * 
     * @param f A file object to use for serialization.
     * @param bufferSize Size of the internal serializer buffer.
     */
    InputFileSerializer(const File &f, size_t bufferSize)
    :   InputRandomAccessSerializer(bufferSize),
        _f(f)
    { }

    /**
     * @brief Construct a new InputFileSerializer object.
     * 
     * @param f A file object to use for serialization.
     * @param bufferSize Size of the internal serializer buffer.
     */
    InputFileSerializer(File &&f, size_t bufferSize)
    :   InputRandomAccessSerializer(bufferSize),
        _f(std::move(f))
    { }
};

}
