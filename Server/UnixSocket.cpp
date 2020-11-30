#include "UnixSocket.h"
#include "Server.h"
#include <glog/logging.h>
#include <boost/asio/local/connect_pair.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/basic_socket.hpp>
#include <boost/asio/read.hpp>

namespace IOEvent
{


namespace UnixSocket
{

static void setUnixSocketBlock(boost::asio::local::stream_protocol::socket *socket)
{
    boost::asio::socket_base::bytes_readable command(false);
    socket->io_control(command);
}

static void setUnixSocketNonblock(boost::asio::local::stream_protocol::socket *socket)
{
    boost::asio::socket_base::bytes_readable command(true);
    socket->io_control(command);
}

static void setUnixSocketBufferSize(boost::asio::local::stream_protocol::socket *socket, int32_t size)
{
    boost::asio::socket_base::receive_buffer_size recv_size_op(size);
    boost::asio::socket_base::send_buffer_size send_size_op(size);
    socket->set_option(recv_size_op);
    socket->set_option(send_size_op);
}

static ssize_t UnixSocketRead(Pipe *p, void *_buf, size_t length)
{
    
}

static ssize_t UnixSocketWrite(Pipe *p, const void *_buf, size_t length)
{

}

static boost::asio::local::stream_protocol::socket *UnixSocketGetSocket(Pipe *p, int master) 
{
    return master ? &p->masterPipe_ : &p->workerPipe_;
}

static void UnixSocketClose(Pipe *p) 
{
   p->masterPipe_.close();
   p->workerPipe_.close();
}

bool UnixSocketCreate(Factory *factor)
{
    Server *server = (Server *)factor->ptr;
    Pipe *pipes = factor->pipes;
    std::unique_ptr<PipeUnsock> object = std::unique_ptr<PipeUnsock>();
    auto ret = socketpair(AF_UNIX, SOCK_STREAM, 0, object->socks);
    if (ret < 0) {
        LOG(ERROR) << "socketpair() failed";
        return false;
    }

    for(auto i = 0; i < server->workerNum_; ++i)
    {
        factor->pipes[i].blocking = 1;
        factor->pipes[i].object = object.release();
        factor->pipes[i].read = UnixSocketRead;
        factor->pipes[i].write = UnixSocketWrite;
        factor->pipes[i].getSocket = UnixSocketGetSocket;
        factor->pipes[i].close = UnixSocketClose;
        factor->pipes[i].setBlock = setUnixSocketBlock;
        factor->pipes[i].setNonblock = setUnixSocketNonblock;
    }
    return true;
}

// bool UnixSocketCreate(Factory *factor)
// {
//     Server *server = (Server *)factor->ptr;
//     Pipe *pipes = factor->pipes;
//     pipes = new Pipe[server->workerNum_]();
//     for(auto i = 0; i < server->workerNum_; ++i)
//     {
//         pipes[i].blocking = 1;
//         boost::asio::local::connect_pair(pipes[i].workerPipe_, pipes[i].masterPipe_);    
//         setUnixSocketBlock(&(pipes[i].masterPipe_));
//         setUnixSocketBlock(&(pipes[i].workerPipe_));
//         // 设置缓冲区大小
//         setUnixSocketBufferSize(&(pipes[i].masterPipe_), UNIXSOCK_MAX_BUF_SIZE);
//         setUnixSocketBufferSize(&(pipes[i].workerPipe_), UNIXSOCK_MAX_BUF_SIZE);
//         pipes[i].read = UnixSocketRead;
//         pipes[i].write = UnixSocketWrite;
//         pipes[i].getSocket = UnixSocketGetSocket;
//         pipes[i].close = UnixSocketClose;
//         pipes[i].setBlock = setUnixSocketBlock;
//         pipes[i].setNonblock = setUnixSocketNonblock;
//     }
//     return true;
// }  

}
}