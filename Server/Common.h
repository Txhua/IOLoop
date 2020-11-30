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

struct PipeUnsock {
    /**
     * master : socks[1]
     * worker : socks[0]
     */
    int socks[2];
    /**
     * master pipe is closed
     */
    bool pipe_master_closed;
    /**
     * worker pipe is closed
     */
    bool pipe_worker_closed;
};

struct Pipe {
    using Socket = boost::asio::local::stream_protocol::socket;
    PipeUnsock *object;
    int blocking;
    double timeout;
    Socket masterPipe_;
    Socket workerPipe_;
    ssize_t (*read)(Pipe *, void *_buf, size_t length);
    ssize_t (*write)(Pipe *, const void *_buf, size_t length);
    Socket *(*getSocket)(Pipe *, int master);
    void (*close)(Pipe *);
    void (*setBlock)(Socket *socket);
    void (*setNonblock)(Socket *socket);
};

struct Factory {
    Pipe *pipes;
    void *ptr;  // server object
    int (*start)(Factory *);
    int (*shutdown)(Factory *);
    bool (*dispatch)(Factory *, void *);
    bool (*finish)(Factory *, void *);
    bool (*notify)(Factory *, void *);  // send a event notify
    bool (*end)(Factory *, int fd);
    void (*free)(Factory *);
};

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