#ifndef _IOEVENT_FILE_H
#define _IOEVENT_FILE_H

#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

namespace IOEvent
{
class File final : public boost::noncopyable
{
public:
    using FileStatus = struct stat;
    explicit File(int32_t fd);
    File(int32_t fd, const boost::filesystem::path &path);
    File(const boost::filesystem::path &path, int32_t flags);
    File(const boost::filesystem::path &path, int32_t flags, int32_t mode);
    ~File();
public:
    enum Flag
    {
        READ = O_RDONLY,
        WRITE = O_WRONLY,
        RW = O_RDWR,
        CREATE = O_CREAT,
        EXCL = O_EXCL,
        APPEND = O_APPEND
    };
public:
    bool ready();
    ssize_t write(const void *__buf, size_t __n) const;
    ssize_t read(void *__buf, size_t __n) const;
    ssize_t pwrite(const void *__buf, size_t __n, off_t __offset) const;
    ssize_t pread(void *__buf, size_t __n, off_t __offset) const;
    size_t writeAll(const void *__buf, size_t __n);
    size_t readAll(void *__buf, size_t __n);
    bool stat(FileStatus *_stat);
    bool sync();
    void release();
    int32_t getFd();
    bool truncate(size_t __n);
    const boost::filesystem::path &getPath() const;
    static bool exists(const std::string &file);
    ssize_t getSize();
    bool lock(int32_t operation);
    bool unlock();
    off_t getOffset();
    off_t setOffset(off_t offset);
    bool close();
private:
    ssize_t fileGetSize();
private:
    int32_t fd_;
    int32_t flags_;
    boost::filesystem::path path_;
};
}










#endif // !_IOEVENT_FILE_H