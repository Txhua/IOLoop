#ifndef _IOEVENT_UNIXSOCKET_H
#define _IOEVENT_UNIXSOCKET_H

#include "Common.h"
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/local/stream_protocol.hpp>



namespace IOEvent
{
class SocketPair : boost::noncopyable
{
public:
    SocketPair();
    ~SocketPair();
public:
    bool setBlocking(int32_t nonblock, int32_t cloexec);
    bool initSocket(const boost::asio::io_context &io_c, bool master);
    boost::asio::local::stream_protocol::socket *getSocket(bool master);
    bool setBufferSize(uint32_t _buffer_size);
    bool setRecvBuffSize(uint32_t _buffer_size);
    bool setSendBuffSize(uint32_t _buffer_size);
    bool ready();
protected:
    int32_t socks[2];
    std::unique_ptr<boost::asio::local::stream_protocol::socket> masterSpcket_;
    std::unique_ptr<boost::asio::local::stream_protocol::socket> workerSpcket_;
};


class UnixSocket : public SocketPair
{
public:
    UnixSocket(int32_t _protocol);
private:
    int32_t protocol_;
};


}





#endif // !_IOEVENT_UNIXSOCKET_H