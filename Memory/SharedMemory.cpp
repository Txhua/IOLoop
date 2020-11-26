#include "SharedMemory.h"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SHM_MMAP_FILE_LEN 64

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

struct IOShareMemory 
{
    size_t size;
    char mapfile[SHM_MMAP_FILE_LEN];
    int tmpfd;
    void *memory;
};

static void *shareMemoryMmapCreate(IOShareMemory *object, size_t size, const char *mapfile)
{
    void *memory = nullptr;
    int32_t flag = MAP_SHARED;
    int32_t tmpfd = -1;
    memset(object, 0, sizeof(IOShareMemory));
#ifdef MAP_ANONYMOUS
    flag |= MAP_ANONYMOUS;
#else
    if(mapfile == nullptr)
    {
        mapfile = "/dev/null";
    }
    if((tmpfd = open(mapfile, O_RDWR)) < 0)
    {
        return nullptr;
    }
    strncpy(object->mapfile, mapfile, SHM_MMAP_FILE_LEN);
    object->tmpfd = tmpfd;
#endif
    memory = mmap(nullptr, size, PROT_READ | PROT_WRITE, flag, tmpfd, 0);
    if(memory == MAP_FAILED)
    {
        return nullptr;
    }
    object->size = size;
    object->memory = memory;
    return memory;
}

void *shmMalloc(size_t size)
{
    IOShareMemory object;
    void *memory = nullptr;
    size += sizeof(IOShareMemory);
    memory = shareMemoryMmapCreate(&object, size, nullptr);
    if(!memory){
        return nullptr;
    }
    memcpy(memory, &object, sizeof(IOShareMemory));
    return (char *)memory + sizeof(IOShareMemory);
}

} 
} // ! namespace IOEvent