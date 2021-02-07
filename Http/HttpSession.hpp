#ifndef _IOEVENT_HTTP_SESSION_H
#define _IOEVENT_HTTP_SESSION_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/noncopyable.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/verb.hpp>
#include <glog/logging.h>
#include <unordered_map>
#include <string_view>
#include <memory>
#include <string>
#include <map>

#include "RequestProcessor.hpp"
#include "ShareMutex.hpp"
#include "Connection.hpp"
#include "StrandStream.h"
#include "Cb.hpp"
#include "Timer.hpp"
#include "Regex.h"
#include "Queue.hpp"


#define HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(router) \
    flesh_type(std::declval<socket_type>(), \
               router.resource_map(), \
               router.method_map(), \
               router.regex_flags(), \
               &router.mutex(), \
               std::declval<buffer_type>(), \
               std::declval<OnAction>()...)



#define HTTP_SESSION_TMPL_ATTRIBUTES \
    Body, RequestParse, ResponseSerializer, Buffer, Protocol, \
    Socket, Clock, Timer, Entry, Container, MethodMap, ResourceMap,OnError, OnTimer

#define HTTPSESSION_TMPL_DECLARE \
	typename Body, \
	typename RequestParse,\
	typename ResponseSerializer, \
	typename Buffer, \
	typename Protocol, \
	typename Socket, \
	typename Clock, \
	typename Timer, \
	template <typename> class Entry, \
	template <typename, typename ...> class Container, \
	template <typename Key, typename Value, typename ... Args> class MethodMap, \
	template <typename Key, typename Value, typename ... Args> class ResourceMap, \
	template <typename> class OnError, \
	template <typename> class OnTimer



namespace IOEvent
{

template
<
	typename Body = boost::beast::http::string_body,
	typename RequestParse = boost::beast::http::request_parser<Body>,
	typename ResponseSerializer = boost::beast::http::response_serializer<Body>,
	typename Buffer = boost::beast::flat_buffer,
	typename Protocol = boost::asio::ip::tcp,
	typename Socket = boost::asio::basic_stream_socket<Protocol>,
	typename Clock =std::chrono::steady_clock,
	typename Timer = boost::asio::basic_waitable_timer<Clock>,
	template <typename> class Entry = std::function,
	template <typename, typename ...> class Container = std::vector,
	template <typename Key, typename Value, typename ...Args> class MethodMap = std::map,
	template <typename Key, typename Value, typename ...Args> class ResourceMap = std::unordered_map,
	template <typename> class OnError = std::function, /* On error expired handler */
	template <typename> class OnTimer = std::function  /* On timer expired handler */
>
class HttpSession
	:boost::noncopyable
{
	enum class context_policy { shared, weak };
	using self_type = HttpSession;
	class Flesh;
	template <typename, context_policy>
	class Context;
public:
	struct option
	{
		struct on_error_t
		{
			
		};
		struct on_timer_t
		{
			
		};
		struct on_socket_t
		{
			
		};
	};
	using flesh_type = Flesh;
	using context_type = Context<flesh_type, context_policy::shared>;
	using weak_context = Context<flesh_type, context_policy::weak>;
	using resource_regex_type = std::string;
	using resource_type = std::string_view;
	using method_type = boost::beast::http::verb;
	using status_type = boost::beast::http::status;
	using mutex_type = ShareMutex::mutex_type;
	using body_type = Body;
	using executor_type = Http::executor;
	using request_parse_type = RequestParse;
	using response_serializer_type = ResponseSerializer;
	using request_type = boost::beast::http::request<Body>;
	using buffer_type = Buffer;
	using connection_type = Connection<Socket, StrandStream::asio_type>;
	using socket_type = typename connection_type::socket_type;
	using shutdown_type = typename socket_type::shutdown_type;
	template <typename _Body>
	using response_type = boost::beast::http::response<_Body>;
	using timer_type = Http::Timer<Timer, StrandStream::asio_type>;
	using duration_type = typename timer_type::duration_type;
	using time_point_type = typename timer_type::time_point;
	using clock_type = typename timer_type::clock_type;
	using regex_type = Http::Regex;
	using regex_flag_type = typename regex_type::flag_type;
	using on_error_type = OnError<void(const boost::system::error_code&, std::string_view)>;
	using on_timer_type = OnTimer<void(context_type)>;
	using storage_type = Http::Storage<self_type, Entry, Container>;
	using queue_type = Http::Queue<Flesh>;
	using resource_map_type = ResourceMap<resource_regex_type, storage_type>; //std::unordered_map<std::string, Http::Storage<self_type, Entry, Container>>
	using method_map_type = MethodMap<method_type, resource_map_type>; // std::map<method_type, std::unordered_map>

	static constexpr typename option::on_error_t on_error_arg{};
	static constexpr typename option::on_timer_t on_timer_arg{};
	static constexpr typename option::on_socket_t get_socket_arg{};
private:
	class Flesh
		: public StrandStream,Http::RequestProcessor<self_type>,
		public std::enable_shared_from_this<Flesh>,
		boost::noncopyable
	{
		friend queue_type;
		friend context_type;
		using flesh_self_type = Flesh;
		using base_type = Http::RequestProcessor<self_type>;
	public:
		flesh_self_type& recv();
		
		flesh_self_type& recv(duration_type const duration);
		
		flesh_self_type& recv(time_point_type const time_point);
		
		template <typename _Body>
		flesh_self_type& send(response_type<_Body>& response);
		
		template <typename _Body>
		flesh_self_type& send(response_type<_Body>& response, duration_type const duration);
		
		template <typename _Body>
		flesh_self_type& send(response_type<_Body>& response, time_point_type const time_point);

		flesh_self_type& wait();

		flesh_self_type& wait(time_point_type const time_point);

		flesh_self_type& wait(duration_type const duration);

		template <typename _Body>
		flesh_self_type& push(response_type<_Body>& response);

		flesh_self_type& timerCancel();

		void eof();

		void cls();
		
		template <typename OnHandler>
		void member(typename option::on_error_t arg,
					OnHandler &handler,
					typename std::enable_if<TryInvoke<OnHandler, void(const boost::system::error_code&, std::string_view)>::value, int>::type = 0);

		template <typename OnHandler>
		void member(typename option::on_timer_t arg,
					OnHandler &handler,
					typename std::enable_if<TryInvoke<OnHandler, void(context_type)>::value, int>::type = 0);

		socket_type& member(typename option::on_socket_t arg);
	public:
		explicit Flesh(socket_type&& socket, std::shared_ptr<resource_map_type> const& resource_map,
					   std::shared_ptr<method_map_type> const& method_map, regex_flag_type regex_flag,
					   mutex_type* mutex,
					   buffer_type&& buffer);
				
						   
		
		template <typename _OnError>
		explicit Flesh(socket_type&& socket, std::shared_ptr<resource_map_type> const& resource_map,
					   std::shared_ptr<method_map_type> const& method_map, regex_flag_type regex_flag,
					   mutex_type* mutex,
					   buffer_type&& buffer,
					   _OnError&& on_error,
					   typename std::enable_if<TryInvoke<_OnError, void(boost::system::error_code, std::string_view)>::value, int>::type = 0);
			
		
		template <typename _OnError, typename _OnTimer>
		explicit Flesh(socket_type&& socket, std::shared_ptr<resource_map_type> const& resource_map,
					   std::shared_ptr<method_map_type> const& method_map, regex_flag_type regex_flag,
					   mutex_type* mutex,
					   buffer_type&& buffer,
					   _OnError&& on_error,
					   _OnTimer&& on_timer,
					   typename std::enable_if<TryInvoke<_OnError, void(boost::system::error_code, std::string_view)>::value
					   && TryInvoke<_OnTimer, void(context_type)>::value, int>::type = 0);

		~Flesh() { LOG(WARNING) << "~Flesh"; }
	private:
		void doRead();
		// ! ´¦Àírequest
		void doProcessRequest();
		void doLaunchTimer();
		void onTimer(const boost::system::error_code &error);
		void doEof(shutdown_type type);
		void doCls();
		template <typename _Body>
		void doWrite(response_type<_Body>& response);
	private:
		timer_type timer_;
		queue_type queue_;
		buffer_type buffer_;
		mutex_type* router_mutex_;
		connection_type connection_;
		on_error_type on_error_;
		on_timer_type on_timer_;
		std::optional<request_parse_type> parser_;
		std::optional<response_serializer_type> serializer_;
	};

	template <typename _Flesh, context_policy policy>
	class Context
	{
		std::shared_ptr<_Flesh> flesh_;
	public:
		Context(_Flesh &flesh)
			:flesh_{ flesh.shared_from_this() }
		{}

		weak_context weak() const
		{
			return weak_context(flesh_);
		}

		socket_type &asio_socket() const
		{
			return advance(get_socket_arg);
		}
		
		void recv() const
		{
			boost::asio::dispatch(static_cast<StrandStream&>(*flesh_).get_inner_executor(),
								  std::bind(static_cast<_Flesh & (_Flesh::*)()>(&_Flesh::recv), flesh_->shared_from_this()));
		}

		void recv(duration_type const duration) const
		{
			boost::asio::dispatch(static_cast<StrandStream&>(*flesh_).get_inner_executor(),
								  std::bind(static_cast<_Flesh & (_Flesh::*)(duration_type const)>(&_Flesh::recv),
											flesh_->shared_from_this(), duration));
		}

		void recv(time_point_type const time_point) const
		{
			boost::asio::dispatch(static_cast<StrandStream&>(*flesh_).get_inner_executor(),
								  std::bind(static_cast<_Flesh & (_Flesh::*)(time_point_type const)>(&_Flesh::recv),
											flesh_->shared_from_this(), time_point));
		}

		template <typename _Body>
		void send(response_type<_Body> &response) const
		{
			boost::asio::dispatch(static_cast<StrandStream&>(*flesh_).get_inner_executor(),
								  std::bind(static_cast<_Flesh & (_Flesh::*)(response_type<_Body> &)>(_Flesh::send),
											flesh_->shared_from_this(), response));
								  
		}

		template <typename _Body>
		void send(response_type<_Body> &&response) const
		{
			boost::asio::dispatch(static_cast<StrandStream&>(*flesh_).get_inner_executor(),
								  std::bind(static_cast<_Flesh & (_Flesh::*)(response_type<_Body> &)>(_Flesh::send),
											flesh_->shared_from_this(), std::move(response)));
		}

		template <typename _Body>
		void send(response_type<_Body>& response, duration_type const duration) const
		{
			boost::asio::dispatch(static_cast<StrandStream&>(*flesh_).get_inner_executor(),
								  std::bind(static_cast<_Flesh & (_Flesh::*)(response_type<_Body> &, duration_type const)>(_Flesh::send),
											flesh_->shared_from_this(), response, duration));

		}

		template <typename _Body>
		void send(response_type<_Body>&& response, duration_type const duration) const
		{
			boost::asio::dispatch(static_cast<StrandStream&>(*flesh_).get_inner_executor(),
								  std::bind(static_cast<_Flesh & (_Flesh::*)(response_type<_Body> &, duration_type const)>(_Flesh::send),
											flesh_->shared_from_this(), std::move(response), duration));

		}

		template <typename _Body>
		void send(response_type<_Body>& response, time_point_type const time_point)
		{
			boost::asio::dispatch(static_cast<StrandStream&>(*flesh_).get_inner_executor(),
								  std::bind(static_cast<_Flesh & (_Flesh::*)(response_type<_Body> &, time_point_type const)>(_Flesh::send),
											flesh_->shared_from_this(), response, time_point));

		}

		template <typename _Body>
		void send(response_type<_Body>&& response, time_point_type const time_point) const
		{
			boost::asio::dispatch(static_cast<StrandStream&>(*flesh_).get_inner_executor(),
								  std::bind(static_cast<_Flesh & (_Flesh::*)(response_type<_Body> &, time_point_type const)>(_Flesh::send),
											flesh_->shared_from_this(), std::move(response), time_point));

		}

		void timer_cancel() const
		{
			boost::asio::dispatch(static_cast<StrandStream&>(*flesh_).get_inner_executor(),
								  std::bind(&_Flesh::timerCancel, flesh_->shared_from_this()));
		}

		void eof() const
		{
			boost::asio::dispatch(static_cast<StrandStream&>(*flesh_).get_inner_executor(),
								  std::bind(&_Flesh::eof, flesh_->shared_from_this()));
		}

		void cls() const
		{
			boost::asio::dispatch(static_cast<StrandStream&>(*flesh_).get_inner_executor(),
								  std::bind(&_Flesh::cls, flesh_->shared_from_this()));
		}

		template <typename Handler>
		auto advance(typename option::on_error_t arg, Handler &&handler) const
			-> decltype(std::declval<Context<_Flesh, context_policy::shared>>().flesh_->member(std::declval<typename option::on_error_t>(), std::declval<Handler>()))
		{
			using handler_type = typename std::decay<Handler>::type;
			boost::asio::dispatch(static_cast<StrandStream&>(*flesh_).get_inner_executor(),
								  std::bind(static_cast<void(Flesh::*)(typename option::on_error_t, handler_type&,
																	   typename std::enable_if<TryInvoke<Handler,
																	   void(context_type)>::value,
																	   int>::type)>(&_Flesh::template member<handler_type>),
											flesh_->shared_from_this(), arg, std::forward<Handler>(handler), 0));
		}

		template <typename Handler>
		auto advance(typename option::on_timer_t arg, Handler &&handler) const
			->decltype(std::declval<Context<_Flesh, context_policy::shared>>().flesh_->member(std::declval<typename option::on_timer_t>(), std::declval<Handler>()))
		{
			using handler_type = typename std::decay<Handler>::type;
			boost::asio::dispatch(static_cast<StrandStream&>(*flesh_).get_inner_executor(),
								  std::bind(static_cast<void(Flesh::*)(typename option::on_timer_t, Handler&,
																	   typename std::enable_if<TryInvoke<Handler,
																	   void(const boost::system::error_code&, std::string_view)>::value, 
																	   int>::type)>(&_Flesh::template member<handler_type>),
											flesh_->shared_from_this(), arg, std::forward<Handler>(handler), 0));
		}

		auto advance(typename option::on_socket_t arg) const
			->decltype(std::declval<Context<_Flesh, context_policy::shared>>().flesh_->member(std::declval<typename option::on_socket_t>()))
		{
			return flesh_->member(arg);
		}
	};

	template <typename _Flesh>
	class Context<_Flesh, context_policy::weak>
	{
		std::weak_ptr<_Flesh> flesh_;
	public:
		Context(std::shared_ptr<_Flesh> flesh)
			:flesh_{ flesh }
		{}

		bool expired() const
		{
			return flesh_->expired();
		}

		Context<_Flesh, context_policy::shared> load() const
		{
			return Context<_Flesh, context_policy::shared>(flesh_.lock());
		}
	};
public:
	template <typename Router, typename ...OnAction>
	static auto recv(socket_type&& socket, Router const& router, buffer_type&& buffer, OnAction&&...on_action)
		-> typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const &>()), std::declval<
			context_type>())>::type;

	template <typename Router, typename ...OnAction>
	static auto recv(socket_type&& socket, Router const& router, OnAction&&...on_action)
		-> typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const&>()), 
										std::declval<context_type>())>::type;

	template <typename Router, typename TimePointOrDuration, typename ...OnAction>
	static auto recv(socket_type&& socket, Router const& router,
					 const TimePointOrDuration time_point_or_duration,
					 buffer_type&& buffer, OnAction&&...on_action)
		-> typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<const Router&>()).recv(std::declval<TimePointOrDuration>()),
										std::declval<context_type>())>::type;

	template <typename Router, typename TimePointOrDuration, typename ...OnAction>
	static auto recv(socket_type&& socket, Router const& router, TimePointOrDuration const time_point_or_duration, OnAction&&...on_action)
		->typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const&>()).recv(std::declval<TimePointOrDuration>()), 
									   std::declval<context_type>())>::type;

	template <typename Router, typename Response, typename ...OnAction>
	static auto send(socket_type&& socket, Router const& router, Response&& response, OnAction &&...on_action)
		->typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const&>()).send(std::declval<Response>()), 
									   std::declval<context_type>())>::type;
	template <typename Router, typename Response, typename ...OnAction>
	static auto send(socket_type&& socket, Router const& router, Response&& response, buffer_type&& buffer, OnAction &&...on_action)
		->typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const&>()).send(std::declval<Response>()),
									   std::declval<context_type>())>::type;
	
	template <typename Router, typename Response, typename TimePointOrDuration, typename ...OnAction>
	static auto send(socket_type&& socket, Router const& router, Response&& response, buffer_type &&buffer, TimePointOrDuration const time_point_or_duration,OnAction &&...on_action)
		->typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const&>()).send(std::declval<Response>(), std::declval<TimePointOrDuration>()),
									   std::declval<context_type>())>::type;

	template <typename Router, typename Response, typename TimePointOrDuration, typename ...OnAction>
	static auto send(socket_type&& socket, Router const& router, Response&& response, TimePointOrDuration const time_point_or_duration, OnAction &&...on_action)
		->typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const&>()).send(std::declval<Response>(), std::declval<TimePointOrDuration>()),
									   std::declval<context_type>())>::type;

	template <typename Router, typename ...OnAction>
	static auto make(socket_type&& socket, Router const& router, OnAction &&...on_action)
		->typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const&>()), std::declval<context_type>())>::type;

	template <typename Router, typename ...OnAction>
	static auto make(socket_type&& socket, Router const& router, buffer_type &&buffer, OnAction &&...on_action)
		->typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const&>()), std::declval<context_type>())>::type;
	
};

template <HTTPSESSION_TMPL_DECLARE>
typename HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::flesh_self_type& HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::recv()
{
	doRead();
	return *this;
}

template <HTTPSESSION_TMPL_DECLARE>
typename HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::flesh_self_type& HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::recv(
	duration_type const duration)
{
	timer_.stream().expires_after(duration);
	doLaunchTimer();
	doRead();
	return *this;
}

template <HTTPSESSION_TMPL_DECLARE>
typename HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::flesh_self_type& HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::recv(
	time_point_type const time_point)
{
	timer_.stream().expires_after(time_point);
	doLaunchTimer();
	doRead();
	return *this;
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename _Body>
typename HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::flesh_self_type&
HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::send(response_type<_Body>& response)
{
	queue_(response);
	return *this;
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename _Body>
typename HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::flesh_self_type&
HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::send(response_type<_Body>& response, duration_type const duration)
{
	timer_.stream().expires_after(duration);
	doLaunchTimer();
	queue_(response);
	return *this;
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename _Body>
typename HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::flesh_self_type&
HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::send(response_type<_Body>& response, time_point_type const time_point)
{
	timer_.stream().expires_after(time_point);
	doLaunchTimer();
	queue_(response);
	return *this;
}

template <HTTPSESSION_TMPL_DECLARE>
typename HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::flesh_self_type& HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::wait()
{
	timer_.stream().expires_after((time_point_type::max)());
	doLaunchTimer();
	return *this;
}

template <HTTPSESSION_TMPL_DECLARE>
typename HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::flesh_self_type& HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::wait(
	time_point_type const time_point)
{
	timer_.stream().expires_after(time_point);
	doLaunchTimer();
	return *this;
}

template <HTTPSESSION_TMPL_DECLARE>
typename HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::flesh_self_type& HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::wait(
	duration_type const duration)
{
	timer_.stream().expires_after(duration);
	doLaunchTimer();
	return *this;
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename _Body>
typename HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::flesh_self_type& HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::push(
	response_type<_Body>& response)
{
	serializer_.emplace();
	auto ec = connection_.write(*serializer_);
	if (ec) 
	{
		if (on_error_) on_error_(ec, "write/push");
	}
	return *this;
}

template <HTTPSESSION_TMPL_DECLARE>
typename HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::flesh_self_type& HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::timerCancel()
{
	auto ec = timer_.cancel();

	if (ec && on_error_) on_error_(ec, "cancel/timerCancel");
	return *this;
}

template <HTTPSESSION_TMPL_DECLARE>
void HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::eof()
{
	doEof(shutdown_type::shutdown_send);
}

template <HTTPSESSION_TMPL_DECLARE>
void HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::cls()
{
	doCls();
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename OnHandler>
void HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::member(typename option::on_error_t arg, OnHandler &handler,
	typename std::enable_if<TryInvoke<OnHandler, void(const boost::system::error_code&, std::string_view)>::value, int>::type)
{
	on_error_ = on_error_type(std::move(handler));
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename OnHandler>
void HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::member(typename option::on_timer_t arg, OnHandler &handler,
	typename std::enable_if<TryInvoke<OnHandler, void(context_type)>::value, int>::type)
{
	on_timer_ = on_timer_type(std::move(handler));
}

template <HTTPSESSION_TMPL_DECLARE>
typename HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::socket_type& HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::member(
	typename option::on_socket_t arg)
{
	return connection_.asio_socket();
}


template <HTTPSESSION_TMPL_DECLARE>
HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::Flesh(socket_type&& socket,
	std::shared_ptr<resource_map_type> const& resource_map, std::shared_ptr<method_map_type> const& method_map,
	regex_flag_type regex_flag, mutex_type* mutex, buffer_type&& buffer)
		:StrandStream{socket.get_executor() },
		base_type{ resource_map, method_map, regex_flag },
		router_mutex_{ mutex },
		timer_{ static_cast<StrandStream&>(*this), (time_point_type::max)() },
		connection_{ std::move(socket), static_cast<StrandStream&>(*this) },
		buffer_{ buffer },
		queue_{ *this }
{
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename _OnError>
HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::Flesh(socket_type&& socket,
	std::shared_ptr<resource_map_type> const& resource_map, std::shared_ptr<method_map_type> const& method_map,
	regex_flag_type regex_flag, mutex_type* mutex, buffer_type&& buffer, _OnError&& on_error,
	typename std::enable_if<TryInvoke<_OnError, void(boost::system::error_code, std::string_view)>::value, int>::type)
	:StrandStream{ socket.get_executor() },
	base_type{ resource_map, method_map, regex_flag },
	router_mutex_{ mutex },
	timer_{ static_cast<StrandStream&>(*this), (time_point_type::max)() },
	connection_{ std::move(socket), static_cast<StrandStream&>(*this) },
	buffer_{ buffer },
	on_error_{ on_error },
	queue_{ *this }
{
}


template <HTTPSESSION_TMPL_DECLARE>
template <typename _OnError, typename _OnTimer>
HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::Flesh(socket_type&& socket,
	std::shared_ptr<resource_map_type> const& resource_map, std::shared_ptr<method_map_type> const& method_map,
	regex_flag_type regex_flag, mutex_type* mutex, buffer_type&& buffer, _OnError&& on_error, _OnTimer&& on_timer,
	typename std::enable_if<TryInvoke<_OnError, void(boost::system::error_code, std::string_view)>::value && TryInvoke<
	_OnTimer, void(context_type)>::value, int>::type)
	:StrandStream{ socket.get_executor() },
	base_type{ resource_map, method_map, regex_flag },
	router_mutex_{ mutex },
	timer_{ static_cast<StrandStream&>(*this), (time_point_type::max)() },
	connection_{ std::move(socket), static_cast<StrandStream&>(*this) },
	buffer_{ buffer },
	on_error_{ on_error },
	on_timer_{ on_timer },
	queue_{ *this }
{
}

template <HTTPSESSION_TMPL_DECLARE>
void HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::doRead()
{
	parser_.emplace();
	auto self = this->shared_from_this();
	connection_.async_read(buffer_, *parser_, std::bind([&, self](const boost::system::error_code& error, size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);
		if (error)
		{
			if (on_error_)(on_error_(error, "async_read/on_read"));
			return;
		}

		doProcessRequest();

	}, std::placeholders::_1, std::placeholders::_2));
}



template <HTTPSESSION_TMPL_DECLARE>
void HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::doProcessRequest()
{
	request_type request = parser_->release();
	{
		HTTP_SHARED_MUTEX_ENTER_TO_READ(*router_mutex_);
		this->provide(request, *this);
	}
	if (!queue_.is_full() && connection_.stream().is_open())
	{
		recv();
	}
}

template <HTTPSESSION_TMPL_DECLARE>
void HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::doLaunchTimer()
{
	timer_.async_wait(std::bind(&Flesh::onTimer, this->shared_from_this(), std::placeholders::_1));
}

template <HTTPSESSION_TMPL_DECLARE>
void HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::onTimer(const boost::system::error_code& error)
{
	context_type ctx{ *this };
	if(error && error != boost::asio::error::operation_aborted)
	{
		if (on_error_) on_error_(error, "async_wait/onTimer");
		return;
	}

	if(timer_.stream().expiry() <= clock_type::now())
	{
		bool is_alive = connection_.stream().is_open();
		if(on_timer_ && is_alive)
		{
			on_timer_(std::move(ctx));
			return;
		}
		if(!is_alive)
		{
			return;
		}

		auto ec = connection_.shutdown(shutdown_type::shutdown_both);
		if (ec && on_error_) on_error_(ec, "shutdown/on_timer");
		ec = connection_.close();
		if (ec && on_error_) on_error_(ec, "close/on_timer");	
	}
}

template <HTTPSESSION_TMPL_DECLARE>
void HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::doEof(shutdown_type type)
{
	if (!connection_.stream().is_open())
	{
		return;
	}

	auto ec = connection_.shutdown(type);
	if (ec && on_error_)
	{
		on_error_(ec, "shutdown/eof");
	}
}

template <HTTPSESSION_TMPL_DECLARE>
void HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::doCls()
{
	if (! connection_.stream().is_open()) return;

	auto ec = connection_.close();
	if (ec && on_error_) on_error_(ec, "close/cls");
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename _Body>
void HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::Flesh::doWrite(response_type<_Body>& response)
{
	serializer_.emplace(response);
	connection_.async_write([&](const boost::system::error_code& ec, std::size_t bytes_transferred, bool close)
	{
		boost::ignore_unused(bytes_transferred);

		if (ec) 
		{
			if (on_error_) on_error_(ec, "async_write/doWrite");

			return;
		}

		if (close) 
		{
			doEof(shutdown_type::shutdown_both);
			return;
		}

		if (queue_.on_write())
		{
			recv();
		}
	});
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename Router, typename ... OnAction>
auto HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::recv(socket_type&& socket, Router const& router, buffer_type&& buffer, OnAction&&... on_action)
	-> typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const &>()), std::declval<context_type>())>::type
{

	context_type ctx(*std::make_shared<flesh_type>(std::move(socket),
												  router.resource_map(),
												  router.method_map(),
												  router.regex_flags(),
												  &router.mutex(),
												  std::move(buffer),
												  std::forward<OnAction>(on_action)...));
	ctx.recv();
	return ctx;
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename Router, typename ... OnAction>
auto HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::recv(socket_type&& socket, Router const& router,
	OnAction&&... on_action) -> typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const &>()), std::declval<
context_type>())>::type
{
	buffer_type buffer;
	context_type ctx(*std::make_shared<flesh_type>(std::move(socket),
												   router.resource_map(),
												   router.method_map(),
												   router.regex_flags(),
												   &router.mutex(),
												   std::move(buffer),
												   std::forward<OnAction>(on_action)...));
	ctx.recv();
	return ctx;
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename Router, typename TimePointOrDuration, typename ... OnAction>
auto HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::recv(socket_type&& socket, Router const& router,
	const TimePointOrDuration time_point_or_duration, buffer_type&& buffer,
	OnAction&&... on_action) -> typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const &>()).recv(std::declval<TimePointOrDuration>()), 
															 std::declval<context_type>())>::type
{
	context_type ctx(*std::make_shared<flesh_type>(std::move(socket),
												   router.resource_map(),
												   router.method_map(),
												   router.regex_flags(),
												   &router.mutex(),
												   std::move(buffer),
												   std::forward<OnAction>(on_action)...));
	ctx.recv(time_point_or_duration);
	return ctx;
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename Router, typename TimePointOrDuration, typename ... OnAction>
auto HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::recv(socket_type&& socket, Router const& router,
	TimePointOrDuration const time_point_or_duration,
	OnAction&&... on_action) -> typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const&>()).recv(std::declval<TimePointOrDuration>()),
															 std::declval<context_type>())>::type
{
	buffer_type buffer;
	context_type ctx(*std::make_shared<flesh_type>(std::move(socket),
												   router.resource_map(),
												   router.method_map(),
												   router.regex_flags(),
												   &router.mutex(),
												   std::move(buffer),
												   std::forward<OnAction>(on_action)...));
	ctx.recv(time_point_or_duration);
	return ctx;
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename Router, typename Response, typename ... OnAction>
auto HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::send(socket_type&& socket, Router const& router, Response&& response,
	OnAction&&... on_action) -> typename std::decay<decltype(
		HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const &>()).send(std::declval<Response>()),
		std::declval<context_type>())>::type
{
	buffer_type buffer;
	context_type ctx(*std::make_shared<flesh_type>(std::move(socket),
												   router.resource_map(),
												   router.method_map(),
												   router.regex_flags(),
												   &router.mutex(),
												   std::move(buffer),
												   std::forward<OnAction>(on_action)...));
	ctx.send(response);
	return ctx;
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename Router, typename Response, typename ... OnAction>
auto HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::send(socket_type&& socket, Router const& router, Response&& response,
	buffer_type&& buffer, OnAction&&... on_action) -> typename std::decay<decltype(
		HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const&>()).send(std::declval<Response>()), 
		std::declval<context_type>())>::type
{
	context_type ctx(*std::make_shared<flesh_type>(std::move(socket),
												   router.resource_map(),
												   router.method_map(),
												   router.regex_flags(),
												   &router.mutex(),
												   std::move(buffer),
												   std::forward<OnAction>(on_action)...));
	ctx.send(response);
	return ctx;
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename Router, typename Response, typename TimePointOrDuration, typename ... OnAction>
auto HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::send(socket_type&& socket, Router const& router, Response&& response,
	buffer_type&& buffer, TimePointOrDuration const time_point_or_duration, OnAction&&... on_action) -> typename std::decay<decltype(
		HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const&>()).send(std::declval<Response>(),std::declval<TimePointOrDuration>()), 
		std::declval<context_type>())>::type
{
	context_type ctx(*std::make_shared<flesh_type>(std::move(socket),
												   router.resource_map(),
												   router.method_map(),
												   router.regex_flags(),
												   &router.mutex(),
												   std::move(buffer),
												   std::forward<OnAction>(on_action)...));
	ctx.send(response, time_point_or_duration);
	return ctx;
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename Router, typename Response, typename TimePointOrDuration, typename ... OnAction>
auto HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::send(socket_type&& socket, Router const& router, Response&& response,
													 TimePointOrDuration const time_point_or_duration, OnAction&&... on_action) -> typename std::decay<decltype(
		HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const&>()).send(std::declval<Response>(),std::declval<TimePointOrDuration>()) ,
		std::declval<context_type>())>::type
{
	buffer_type buffer;
	context_type ctx(*std::make_shared<flesh_type>(std::move(socket),
												   router.resource_map(),
												   router.method_map(),
												   router.regex_flags(),
												   &router.mutex(),
												   std::move(buffer),
												   std::forward<OnAction>(on_action)...));
	ctx.send(response, time_point_or_duration);
	return ctx;
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename Router, typename ... OnAction>
auto HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::make(socket_type&& socket, Router const& router,
	OnAction&&... on_action) -> typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const&>()), 
															 std::declval<context_type>())>::type
{
	buffer_type buffer;
	context_type ctx(*std::make_shared<flesh_type>(std::move(socket),
												   router.resource_map(),
												   router.method_map(),
												   router.regex_flags(),
												   &router.mutex(),
												   std::move(buffer),
												   std::forward<OnAction>(on_action)...));
	return ctx;
}

template <HTTPSESSION_TMPL_DECLARE>
template <typename Router, typename ... OnAction>
auto HttpSession<HTTP_SESSION_TMPL_ATTRIBUTES>::make(socket_type&& socket, Router const& router, buffer_type&& buffer,
	OnAction&&... on_action) -> typename std::decay<decltype(HTTP_SESSION_TRY_INVOKE_FLESH_TYPE(std::declval<Router const&>()), 
															 std::declval<context_type>())>::type
{
	context_type ctx(*std::make_shared<flesh_type>(std::move(socket),
												   router.resource_map(),
												   router.method_map(),
												   router.regex_flags(),
												   &router.mutex(),
												   std::move(buffer),
												   std::forward<OnAction>(on_action)...));
	return ctx;
}

namespace _Default
{
using http_session = HttpSession<>;
} 

}// !IOEvent







#endif // !_IOEVENT_HTTP_SESSION_H