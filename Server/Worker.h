#ifndef _IOEVENT_WORKER_H
#define _IOEVENT_WORKER_H

#include <boost/noncopyable.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <memory>
#include <atomic>


namespace IOEvent
{
class IOLoop;
struct Pipe;
class Worker final: public boost::noncopyable
{
public:
    Worker();
    ~Worker();
public:
    void start();
    void setProcessId(pid_t process){ pid_ = process; }
    void setWorkerId(int32_t id) { id_ = id;}
    void setProcessType(uint8_t type) { type_ = type; }; 
    pid_t getProcessId() const { return pid_; }
    int32_t getWorkerId() const { return id_; }
    void read();
    void write();
public:
    //worker process
    pid_t pid_;
    // worker id
    int32_t id_;
    // 
    uint8_t type_;
    /**
     * master : socks[1]
     * worker : socks[0]
     */
    boost::asio::local::stream_protocol::socket *masterPipe_;
    boost::asio::local::stream_protocol::socket *workerPipe_;
private:
    std::atomic<bool> started_;
    std::unique_ptr<IOLoop> loop_;
};

}








#endif // !_IOEVENT_WORKER_H