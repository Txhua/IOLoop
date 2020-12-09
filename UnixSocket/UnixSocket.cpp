#include "UnixSocket.h"
#include "Server.h"
#include <glog/logging.h>
#include <boost/asio/local/connect_pair.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/basic_socket.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/local/datagram_protocol.hpp>
#include <Types.h>

namespace IOEvent
{

static bool set_sock_blocking(int32_t sock, int32_t nonblock, int32_t cloexec)
{
    int opts, ret;

    if (nonblock >= 0) 
    {
        do {
            opts = fcntl(sock, F_GETFL);
        } while (opts < 0 && errno == EINTR);

        if (opts < 0) 
        {
            LOG(WARNING) << "fcntl F_GETFL failed" <<  sock;
            return false;
        }

        if (nonblock) 
        {
            opts = opts | O_NONBLOCK;
        } 
        else 
        {
            opts = opts & ~O_NONBLOCK;
        }

        do {
            ret = fcntl(sock, F_SETFL, opts);
        } while (ret < 0 && errno == EINTR);

        if (ret < 0) 
        {
            LOG(WARNING) << "fcntl SETFL opts failed" << sock;
            return false;
        }
    }

#ifdef FD_CLOEXEC
    if (cloexec >= 0) 
    {
        do {
            opts = fcntl(sock, F_GETFD);
        } while (opts < 0 && errno == EINTR);

        if (opts < 0) 
        {
            LOG(WARNING) << "fcntl GETFL failed" << sock;
        }

        if (cloexec)
        {
            opts = opts | FD_CLOEXEC;
        } 
        else 
        {
            opts = opts & ~FD_CLOEXEC;
        }

        do {
            ret = fcntl(sock, F_SETFD, opts);
        } while (ret < 0 && errno == EINTR);

        if (ret < 0) 
        {
            LOG(WARNING) << "fcntl SETFD opts failed" << sock;
            return false;
        }
    }
#endif

    return true;
}


bool static set_option(int32_t fd, int32_t level, int32_t optname, int32_t optval)
{
    return setsockopt(fd, level, optname, &optval, sizeof(optval));
}

SocketPair::SocketPair()
{

}

SocketPair::~SocketPair()
{
    if(masterSpcket_)
    {
        masterSpcket_->close();
    }

    if(workerSpcket_)
    {
        workerSpcket_->close();
    }
}

bool SocketPair::setBlocking(int32_t nonblock, int32_t cloexec)
{
    // worker
    set_sock_blocking(socks[0], nonblock, cloexec);
    // master
    set_sock_blocking(socks[1], nonblock, cloexec);
}



bool SocketPair::initSocket(const boost::asio::io_context &io_c, bool master)
{
    if(master)
    {
        if(!masterSpcket_)
        {
            masterSpcket_ = std::make_unique<boost::asio::local::stream_protocol::socket>(io_c);
            masterSpcket_->assign(boost::asio::local::stream_protocol(), socks[1]);
        }
    }
    else
    {
        if(!workerSpcket_)
        {
            workerSpcket_ = std::make_unique<boost::asio::local::stream_protocol::socket>(io_c);
            workerSpcket_->assign(boost::asio::local::stream_protocol(), socks[1]);
        }
    }
   
}

boost::asio::local::stream_protocol::socket *SocketPair::getSocket(bool master)
{
    return master ? get_pointer(masterSpcket_) : get_pointer(workerSpcket_);
}


bool SocketPair::setBufferSize(uint32_t _buffer_size)
{
    if(!setRecvBuffSize(_buffer_size))
    {
        return false;
    }

    if(!setSendBuffSize(_buffer_size))
    {
        return false;
    }
}

bool SocketPair::setRecvBuffSize(uint32_t _buffer_size)
{
    if (set_option(socks[0], SOL_SOCKET, SO_RCVBUF, _buffer_size) != 0) 
    {
        LOG(WARNING) << "setsockopt SO_RCVBUF failed! " << " fd : " << socks[0] << "buf_size: " <<  _buffer_size);
        return false;
    }

    if (set_option(socks[1], SOL_SOCKET, SO_RCVBUF, _buffer_size) != 0) 
    {
        LOG(WARNING) << "setsockopt SO_RCVBUF failed! " << " fd : " << socks[1] << "buf_size: " <<  _buffer_size);
        return false;
    }
    return true;
}

bool SocketPair::setSendBuffSize(uint32_t _buffer_size)
{
    if (set_option(socks[0], SOL_SOCKET, SO_SNDBUF, _buffer_size) != 0) 
    {
        LOG(WARNING) << "setsockopt SO_SNDBUF failed! " << " fd : " << socks[0] << "buf_size: " <<  _buffer_size);
        return false;
    }

    if (set_option(socks[1], SOL_SOCKET, SO_SNDBUF, _buffer_size) != 0) 
    {
        LOG(WARNING) << "setsockopt SO_SNDBUF failed! " << " fd : " << socks[1] << "buf_size: " <<  _buffer_size);
        return false;
    }
    return true;
}


bool SocketPair::ready() 
{ 
    return masterSpcket_ != nullptr && workerSpcket_ != nullptr; 
}

UnixSocket::UnixSocket(int32_t _protocol)
    :protocol_(_protocol)
{
    if(socketpair(AF_UNIX, protocol_, 0, socks) < 0)
    {
        LOG(ERROR) << "socketpair() failed";
        return;
    }
}

// static void setUnixSocketBlock(boost::asio::local::stream_protocol::socket *socket)
// {
//     boost::asio::socket_base::bytes_readable command(false);
//     socket->io_control(command);
// }

// static void setUnixSocketNonblock(boost::asio::local::stream_protocol::socket *socket)
// {
//     boost::asio::socket_base::bytes_readable command(true);
//     socket->io_control(command);
// }

// static void setUnixSocketBufferSize(boost::asio::local::stream_protocol::socket *socket, int32_t size)
// {
//     boost::asio::socket_base::receive_buffer_size recv_size_op(size);
//     boost::asio::socket_base::send_buffer_size send_size_op(size);
//     socket->set_option(recv_size_op);
//     socket->set_option(send_size_op);
// }



}