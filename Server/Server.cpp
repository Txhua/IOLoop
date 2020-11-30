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

void Server::beforeStart()
{
    factory_.ptr = this;
    workers_ = (Worker *) Memory::ShmCalloc(workerNum_, sizeof(Worker));
    if (!workers_) {
        LOG(FATAL) << "Memory::ShmCalloc() failed ";
    }
    // 进程编号
    for(auto i = 0; i < workerNum_; ++i)
    {
        workers_[i].setWorkerId(i);
        workers_[i].setProcessType(PROCESS_WORKER);
    }

    // 创建unixsocket
    if(!UnixSocket::UnixSocketCreate(&factory_)){
        LOG(FATAL) << "createPipe() failed ";
    }

    for(auto i = 0; i < workerNum_; i++)
    {
        workers_[i].masterPipe_ = factory_.pipes[i].getSocket(&factory_.pipes[i], PIPE_MASTER);
        workers_[i].workerPipe_ = factory_.pipes[i].getSocket(&factory_.pipes[i], PIPE_WORKER);
        workers_[i].pipeObject_ = &factory_.pipes[i];
    }
}

void Server::statrtManageProcess()
{
    auto pid = fork();
    if(pid == 0)
    {
        // 子进程进入循环
          for (auto i = 0; i < workerNum_; ++i) 
          {
            Worker *worker = getWorker(i);
            pid = spawnEventWorker(worker);
            if (pid < 0) {
                LOG(ERROR) << "fork() failed";
            } else {
                worker->id_ = pid;
            }
        }
    }
}

pid_t Server::spawnEventWorker(Worker *worker) 
{
    auto pid = fork();
    // fork() failed
    if (pid < 0) {
        LOG(ERROR) << "Fork Worker failed";
        return -1;
    }
    // worker child processor
    else if (pid == 0) {
        exit(startEentWorker(worker));
    }
    // parent,add to writer
    else {
        return pid;
    }
}




void Server::start()
{
    beforeStart();

}

}