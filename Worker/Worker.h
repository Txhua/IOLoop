#ifndef _IOEVENT_WORKER_H
#define _IOEVENT_WORKER_H

#include <boost/noncopyable.hpp>
#include <memory>
#include <atomic>
#include <boost/asio/local/stream_protocol.hpp>
#include <Buffer.h>

namespace IOEvent
{
class IOLoop;
class Worker final: public boost::noncopyable
{
public:
    Worker();
    ~Worker();
public:
    void start();
    void setProcessId(pid_t process){ pid_ = process; }
    void setWorkerId(int32_t id) { id_ = id;}
    pid_t getProcessId() const { return pid_; }
    int32_t getWorkerId() const { return id_; }
    void read();
    void write();
private:
    //worker process
    pid_t pid_;
    // worker id
    int32_t id_;
    /**
     * master : socks[1]
     * worker : socks[0]
     */
    boost::asio::local::stream_protocol::socket masterPipe_;
    boost::asio::local::stream_protocol::socket workerPipe_;
    std::atomic<bool> started_;
    std::unique_ptr<IOLoop> loop_;
    Buffer inputBuffer_;
	Buffer outputBuffer_;
};

}








#endif // !_IOEVENT_WORKER_H