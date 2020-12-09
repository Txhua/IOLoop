#include "Server.h"
#include "Worker.h"
#include "UnixSocket.h"
#include <glog/logging.h>
#include <TcpServer.h>
#include <IOLoop.h>
#include <SharedMemory.h>


namespace IOEvent
{


Server::Server(IOLoop *loop, const boost::asio::ip::tcp::endpoint &endpoint)
    :tcpServer_(std::make_unique<TcpServer>(loop, endpoint)),
    workerNum_(CPU_NUM),
    reactorNum_(REACTOR_NUM > REACTOR_MAX_THREAD ? REACTOR_MAX_THREAD : REACTOR_NUM),
    pageSize_(PAGE_SIZE)
{
    tcpServer_->setThreadNum(reactorNum_);
}

Server::~Server()
{
   
}

Worker *Server::getWorker(uint16_t worker_id)
{
    if (worker_id < workerNum_) 
    {
        return &(workers_[worker_id]);
    }
    return nullptr;
}

}