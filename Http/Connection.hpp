#ifndef _IOEVENT_HTTP_CONNECTION_H
#define _IOEVENT_HTTP_CONNECTION_H

#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/noncopyable.hpp>

namespace IOEvent
{
namespace Http
{
template <typename Derived, typename CompletionExecutor>
class Connection
{
	using self_type = Connection;
	Derived &derived()
	{
		return static_cast<Derived&>(*this);
	}
public:
	template <typename Function, typename Buffer, typename Parser>
	auto async_read(Buffer& buffer, Parser& parse, Function&& function) ->decltype(void())
	{
		boost::beast::http::async_read(derived().stream(), buffer, parse,
			boost::asio::bind_executor(completion_executor_, std::forward<Function>(function)));
	}

	template <typename Function, typename Serializer>
	auto async_write(Serializer &serializer, Function &&function) ->decltype(void())
	{
		boost::beast::http::async_write(derived().stream(), serializer,
			boost::asio::bind_executor(completion_executor_, std::forward<Function>(function)));
	}

	template <typename Buffer, typename Parser>
	auto read(Buffer &buffer, Parser &parse) -> decltype(boost::system::error_code{})
	{
		boost::system::error_code error;
		boost::beast::http::read(derived().stream(), buffer, parse, error);
		return error;
	}
	template <typename Serializer>
	auto write(Serializer &serializer) -> decltype(boost::system::error_code{})
	{
		boost::system::error_code error;
		boost::beast::http::write(derived().stream(), serializer, error);
		return error;
	}
	
protected:
	explicit Connection(CompletionExecutor const &executor)
		:completion_executor_(executor)
	{}
	CompletionExecutor completion_executor_;
};
}

template <typename Socket, typename CompletionExecutor>
class Connection
	: public Http::Connection<Connection<Socket, CompletionExecutor>, CompletionExecutor>,
	boost::noncopyable
{
	using self_type = Connection;
	using http_connection = Http::Connection<self_type, CompletionExecutor>;
public:
	using socket_type = Socket;
	using shutdown_type = typename socket_type::shutdown_type;
public:
	explicit Connection(socket_type &&socket, const CompletionExecutor &executor)
		:http_connection{ executor },
		socket_{std::move(socket)}
	{}

	auto shutdown(shutdown_type type) -> decltype(boost::system::error_code{})
	{
		auto error = boost::system::error_code{};
		socket_.shutdown(type, error);
		return error;
	}

	auto close() ->decltype(boost::system::error_code{})
	{
		auto error = boost::system::error_code{};
		socket_.close(error);
		return error;
	}

	socket_type &stream()
	{
		return socket_;
	}
	
	socket_type &asio_socket()
	{
		return socket_;
	}
private:
	socket_type socket_;
};
	
}








#endif // !_IOEVENT_HTTP_CONNECTION_H