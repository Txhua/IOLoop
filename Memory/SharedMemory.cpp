#include "SharedMemory.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glog/logging.h>
#include <File.h>


//
// 当使用mmap映射文件或者对象到进程后,
// 就可以直接操作这段虚拟地址进行文件或者对象的读写等操作,
// 不必再调用read,write等系统调用.
// 但需注意,直接对该段内存写时不会写入超过当前文件或者对象大小的内容.
//
namespace IOEvent
{
namespace Memory
{

struct SharedMemory
{
    size_t size_;
    static void *alloc(size_t size);
    static void free(void *ptr);
    static SharedMemory *fetch_object(void *ptr) 
    {
        return (SharedMemory *) ((char *) ptr - sizeof(SharedMemory));
    }
};

void *SharedMemory::alloc(size_t size) 
{
    void *mem;
    int tmpfd = -1;
    int flags = MAP_SHARED;
    SharedMemory object;
    size += sizeof(SharedMemory);

#ifdef MAP_ANONYMOUS
    flags |= MAP_ANONYMOUS;
#else
    File zerofile("/dev/zero", O_RDWR);
    if (!zerofile.ready()) {
        return nullptr;
    }
    tmpfd = zerofile.get_fd();
#endif
    mem = mmap(nullptr, size, PROT_READ | PROT_WRITE, flags, tmpfd, 0);
#ifdef MAP_FAILED
    if (mem == MAP_FAILED)
#else
    if (!mem)
#endif
    {
        LOG(ERROR) << "mmap failed size ： " <<  size;
        return nullptr;
    } else {
        object.size_ = size;
        memcpy(mem, &object, sizeof(object));
        return (char *) mem + sizeof(object);
    }
}


void SharedMemory::free(void *ptr) 
{
    SharedMemory *object = SharedMemory::fetch_object(ptr);
    size_t size = object->size_;
    if (munmap(object, size) < 0) 
    {
        LOG(ERROR)  << "munmap() failed " << object << size;
    }
}

} // ! namespace Memory

using IOEvent::Memory::SharedMemory;

void *shmMalloc(size_t size)
{
    return SharedMemory::alloc(size);
}

void *ShmCalloc(size_t num, size_t _size)
{
    return SharedMemory::alloc(num * _size);
}

void *Realloc(void *ptr, size_t new_size)
{
    SharedMemory *object = SharedMemory::fetch_object(ptr);
    void *new_ptr = shmMalloc(new_size);
    if (new_ptr == nullptr) {
        return nullptr;
    }
    memcpy(new_ptr, ptr, object->size_);
    SharedMemory::free(ptr);
    return new_ptr;
}

int ShmProtect(void *addr, int flags)
{
    SharedMemory *object = SharedMemory::fetch_object(addr);
    return mprotect(object, object->size_, flags);
}

void *ShmMapFree(void *ptr)
{
    SharedMemory::free(ptr);
}


} // ! namespace IOEvent