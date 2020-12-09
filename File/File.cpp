#include "File.h"
#include <glog/logging.h>
#include <sys/file.h>

namespace IOEvent
{
File::File(int32_t fd)
    :fd_(fd),
    flags_(0)
{

}

File::File(int32_t fd, const boost::filesystem::path &path)
    :fd_(fd),
    path_(path),
    flags_(0)
{
    
}

File::File(const boost::filesystem::path &path, int32_t flags)
{
    fd_ = ::open(path.c_str(), flags);
    path_ = path;
    flags_ = flags;
}

File::File(const boost::filesystem::path &path, int32_t flags, int32_t mode)
{
    fd_ = ::open(path.c_str(), flags, mode);
    path_ = path;
    flags_ = flags;
}

File::~File()
{
    if(fd_ >= 0)
    {
        ::close(fd_);
    }
}

bool File::ready()
{
    return fd_ != -1;
}

ssize_t File::write(const void *__buf, size_t __n) const
{
    return ::write(fd_, __buf, __n);
}

ssize_t File::read(void *__buf, size_t __n) const
{
    return ::read(fd_, __buf, __n);
}

ssize_t File::pwrite(const void *__buf, size_t __n, off_t __offset) const
{
    return ::pwrite(fd_, __buf, __n, __offset);
}

ssize_t File::pread(void *__buf, size_t __n, off_t __offset) const
{
    return ::pread(fd_, __buf, __n, __offset);
}

size_t File::writeAll(const void *__buf, size_t __n)
{
    size_t write_bytes = 0;
    while (write_bytes < __n)
    {
        ssize_t n = 0;
        if(flags_ & APPEND)
        {
            n = write((char *)__buf + write_bytes, __n - write_bytes);
        }
        else
        {
           n = pwrite((char *)__buf + write_bytes, __n - write_bytes, write_bytes);
        }

        if(n > 0)
        {
            write_bytes += n;
        }
        else if(n == 0)
        {
            break;
        }
        else
        {
            if (errno == EINTR) 
            {
                continue;
            } 
            else if (!(errno == EAGAIN || errno == EWOULDBLOCK)) 
            {
                LOG(ERROR) << "pwrite failed " << " fd_ : " << fd_ << " __buf : " << __buf << " __n : " << __n - write_bytes << " __offset : " << write_bytes;
            }
            break;
        }    
    }
    return write_bytes;
}
size_t File::readAll(void *__buf, size_t __n)
{
    size_t read_bytes = 0;
    while (read_bytes < __n)
    {
        auto n = pread((char *)__buf + read_bytes, __n - read_bytes, read_bytes);
        if(n > 0)
        {
            read_bytes += n;
        }
        else if(n == 0)
        {
            break;
        }
        else
        {
            if (errno == EINTR) 
            {
                continue;
            } 
            else if (!(errno == EAGAIN || errno == EWOULDBLOCK)) 
            {
                LOG(ERROR) <<"pread failed " << "fd_ : " << fd_ << " __buf : " << __buf << " __n : " << __n - read_bytes << "read_bytes : " << read_bytes;
            }
            break;
        }
        
    }
    return read_bytes;
}

bool File::stat(FileStatus *_stat)
{
    if(::fstat(fd_, _stat) < 0)
    {
        LOG(ERROR) << "fstat() failed";
        return false;
    }
    return true;
}

bool File::sync()
{
    return ::fsync(fd_) == 0;
}

void File::release()
{
    fd_ = -1;
}

int32_t File::getFd()
{
    return fd_;
}

const boost::filesystem::path &File::getPath() const
{
    return path_;
}

bool File::exists(const std::string &file)
{
    return access(file.c_str(), R_OK) == 0;
}

bool File::truncate(size_t __n)
{
    return ::ftruncate(fd_, __n);
}

ssize_t File::getSize()
{
    return fileGetSize();
}

ssize_t File::fileGetSize()
{
    FileStatus file_stat;
    if (fstat(fd_, &file_stat) < 0) {
        LOG(ERROR) << "fstat errno: " << errno;
        return -1;
    }
    if (!S_ISREG(file_stat.st_mode)) {
         LOG(ERROR) << "S_ISREG errno: " << errno;
        return -1;
    }
    return file_stat.st_size;
}

bool File::lock(int32_t operation)
{
    return ::flock(fd_, operation);
}

bool File::unlock()
{
    return ::flock(fd_, LOCK_UN);
}

off_t File::getOffset()
{
    return ::lseek(fd_, 0, SEEK_CUR);
}

off_t File::setOffset(off_t offset)
{
    return ::lseek(fd_, offset, SEEK_SET);
}

bool File::close()
{
    if (fd_ == -1) 
    {
        return false;
    }
    int32_t tmp_fd = fd_;
    fd_ = -1;
    return ::close(tmp_fd) == 0;
}

}