#ifndef _IOEvent_HTTP_SERVER_H
#define _IOEvent_HTTP_SERVER_H

#include <boost/asio/basic_socket_acceptor.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <glog/logging.h>
#include <functional>
#include "StrandStream.h"
#include "Traits.hpp"
#include <string>
#include <atomic>
#include "HttpSession.hpp"
#include "BasicRouter.hpp"
#include "ChainRouter.hpp"

#define HTTP_SERVER_DECLARE \
    template <typename> class OnAccept,\
	template <typename> class OnError,\
	template <typename> class Endpoint,\
	typename Protocol,\
	typename Acceptor,\
	typename Socket

#define HTTP_SERVER_ATTRIBUTES \
    OnAccept, OnError, Endpoint, Protocol, Acceptor, Socket


namespace IOEvent
{
template
<
    template <typename> class OnAction = std::function,
	template <typename> class OnError = std::function,
	template <typename> class Endpoint = boost::asio::ip::basic_endpoint,
	typename Protocol = boost::asio::ip::tcp,
	typename Acceptor = boost::asio::basic_socket_acceptor<Protocol>,
	typename Socket = boost::asio::basic_stream_socket<Protocol>
>
class HttpServer final
	: public StrandStream,
	boost::noncopyable
{
    using HttpSessionMap = std::map<std::string, _Default::http_session>;
public:
    using self_type = HttpServer;
    using acceptor_type = Acceptor;
    using protocol_type = Protocol;
    using socket_type = Socket;
    using io_context = boost::asio::io_context;
    using asio_executor = typename socket_type::executor_type;
    using endpoint_type = Endpoint<protocol_type>;
    using on_action_type = OnAction<void(const socket_type&)>;
    using on_error_type = OnError<void(const boost::system::error_code&, std::string_view)>;
    using session_type = _Default::http_session;
    using basic_router_type = Http::BasicRouter<session_type>;
    using chain_router_type = Http::ChainRouter<session_type>;
public:
    template <typename _OnAction>
    explicit HttpServer(io_context& loop,
        _OnAction&& on_action,
        const std::string& _name = "HttpServer",
        typename std::enable_if<TryInvoke<_OnAction, void(const socket_type&)>::value, int>::type = 0)
        :StrandStream{ loop.get_executor()},
		loop_{ loop },
        acceptor_{ loop_ },
        onActionCallback_{ std::forward<_OnAction>(on_action) },
        name_{ _name },
		start_{false},
		nextConnectionId_{ 1 },
        basicRouter_{ std::regex::ECMAScript },
        chainRouter_{ std::regex::ECMAScript }
	{}

    template <typename _OnAction, typename _OnError>
    explicit HttpServer(io_context& loop,
        _OnAction&& on_action,
        _OnError&& on_error,
        const std::string& _name = "HttpServer",
        typename std::enable_if<TryInvoke<_OnAction, void(socket_type)>::value
        && TryInvoke<_OnError, void(const boost::system::error_code&, std::string_view)>::value, int>::type = 0)
        :StrandStream{ loop.get_executor() },
		loop_{ loop },
        acceptor_{ loop_ },
        onActionCallback_{ std::forward<_OnAction>(on_action) },
        onErrorCallback_{ std::forward<_OnError>(on_error) },
        name_{ _name },
        start_{ false },
        nextConnectionId_{ 1 },
        basicRouter_{ std::regex::ECMAScript },
		chainRouter_{ std::regex::ECMAScript }
    {}
    void start(const endpoint_type& endpoint);

    basic_router_type& basicRouter() { return basicRouter_; }
    chain_router_type& chainRouter() { return chainRouter_; }
	
    ~HttpServer() { LOG(ERROR) << "~HttpServer()"; }
private:
    const std::string& name() const { return name_; }
    void loop(const endpoint_type& endpoint);
    void doAccept();
    void onAccept(socket_type&& socket);
private:
    io_context &loop_;
    uint32_t nextConnectionId_;
    acceptor_type acceptor_;
    endpoint_type endpoint_;
    on_action_type onActionCallback_;
    on_error_type onErrorCallback_;
    const std::string name_;
    std::atomic<bool> start_;
    HttpSessionMap connection_;
    basic_router_type basicRouter_;
    chain_router_type chainRouter_;
};

template<HTTP_SERVER_DECLARE>
void HttpServer<HTTP_SERVER_ATTRIBUTES>::start(const endpoint_type& endpoint)
{
    if (!start_)
    {
        start_ = true;
        loop(endpoint);
    }
}

template<HTTP_SERVER_DECLARE>
void HttpServer<HTTP_SERVER_ATTRIBUTES>::loop(const endpoint_type& endpoint)
{
    auto error = boost::system::error_code{};
    acceptor_.open(endpoint.protocol(), error);
    if (error)
    {
        if (onErrorCallback_) onErrorCallback_(error, "acceptor open error");
        return;
    }

    acceptor_.set_option(boost::asio::socket_base::reuse_address(true), error);
    if (error)
    {
        if (onErrorCallback_) onErrorCallback_(error, "acceptor set_option error");
        return;
    }

    acceptor_.bind(endpoint, error);
    if (error)
    {
        if (onErrorCallback_) onErrorCallback_(error, "acceptor bind error");
        return;
    }

    acceptor_.listen(boost::asio::socket_base::max_listen_connections, error);
    if (error)
    {
        if (onErrorCallback_) onErrorCallback_(error, "acceptor listen error");
        return;
    }

    endpoint_ = endpoint;
    doAccept();
}

template <HTTP_SERVER_DECLARE>
void HttpServer<HTTP_SERVER_ATTRIBUTES>::doAccept()
{
    acceptor_.async_accept([&](const boost::system::error_code& error, socket_type&& socket)
    {
        boost::asio::post(static_cast<StrandStream&>(*this).get_inner_executor(), std::bind([&](const boost::system::error_code& ec, socket_type& sock)
        {
            if (ec)
            {
                if (onErrorCallback_) onErrorCallback_(ec, "accept/do_loop");
                return;
            }

            if (onActionCallback_) onActionCallback_(sock);

            onAccept(std::move(sock));
        }, error, std::move(socket)));

        doAccept();
    });
}

template <HTTP_SERVER_DECLARE>
void HttpServer<HTTP_SERVER_ATTRIBUTES>::onAccept(socket_type&& socket)
{
    LOG(INFO) << socket.remote_endpoint().address().to_string() << ":" << std::to_string(endpoint_.port()) << " connected!";
    _Default::http_session::recv(std::move(socket), basicRouter_, onErrorCallback_);
}




namespace _Default
{
using http_server_type = HttpServer<>;
}

}





#endif // !_IOEvent_HTTP_SERVER_H