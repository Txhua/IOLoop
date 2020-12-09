#ifndef _IOEVENT_SERVER_H
#define _IOEVENT_SERVER_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/noncopyable.hpp>
#include <memory>
#include "Common.h"


namespace IOEvent 
{
class TcpServer;
class IOLoop;
class Worker;
class Server final : public boost::noncopyable
{
public:
   explicit Server(IOLoop *loop, const boost::asio::ip::tcp::endpoint &endpoint);
   ~Server(); 
   Worker *getWorker(uint16_t worker_id);
public:
    //reactor thread/process num
    uint16_t reactorNum_;
    //worker process num
    uint32_t workerNum_;
    uint32_t pageSize_;
private:
    Worker *workers_;
    std::unique_ptr<TcpServer> tcpServer_;
};


}




#endif // !_IOEVENT_SERVER_H