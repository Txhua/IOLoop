#ifndef _IOEVENT_SHARED_MEMORY_H
#define _IOEVENT_SHARED_MEMORY_H

#include <sys/mman.h>
#include <boost/noncopyable.hpp>

#define SHM_MMAP_FILE_LEN 64

namespace IOEvent
{
namespace Memory
{
// 创建共享内存
void *ShmMalloc(size_t size);
// 释放共享内存
void *ShmMapFree(void *ptr);

}

    
} // ! namespace IOEvent











#endif // !_IOEVENT_SHARE_MEMORY_H