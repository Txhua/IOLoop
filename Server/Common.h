#ifndef _IOEVENT_SERVER_COMMON_H
#define _IOEVENT_SERVER_COMMON_H

#include <unistd.h>
#include <boost/asio/local/stream_protocol.hpp>

namespace IOEvent
{

#define _MAX(A, B) ((A) > (B) ? (A) : (B))
#define _MIN(A, B) ((A) < (B) ? (A) : (B))
#define CPU_NUM (_MAX(1, sysconf(_SC_NPROCESSORS_ONLN)))
#define REACTOR_NUM CPU_NUM
#define SW_WORKER_NUM (CPU_NUM * 2)
#define REACTOR_MAX_THREAD 8
#define PAGE_SIZE (getpagesize())
#define UNIXSOCK_MAX_BUF_SIZE (2 * 1024 * 1024)


enum Process_type {
    PROCESS_MASTER  = 1,
    PROCESS_WORKER  = 2,
    PROCESS_MANAGER = 3,
};

enum Pipe_type {
    PIPE_WORKER   = 0,
    PIPE_MASTER   = 1,
    PIPE_READ     = 0,
    PIPE_WRITE    = 1,
    PIPE_NONBLOCK = 2,
};

}



#endif // !_IOEVENT_SERVER_COMMON_H