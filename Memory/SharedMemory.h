#ifndef _IOEVENT_SHARED_MEMORY_H
#define _IOEVENT_SHARED_MEMORY_H

#include <sys/mman.h>

#define SHM_MMAP_FILE_LEN 64

namespace IOEvent
{
void *ShmMalloc(size_t size);
void *ShmCalloc(size_t num, size_t _size);
void *Realloc(void *ptr, size_t new_size);
int ShmProtect(void *addr, int flags);
void *ShmMapFree(void *ptr);
   
} // ! namespace IOEvent











#endif // !_IOEVENT_SHARE_MEMORY_H