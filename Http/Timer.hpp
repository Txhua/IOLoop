#ifndef _IOEVENT_HTTP_TIMER_H
#define _IOEVENT_HTTP_TIMER_H

#include <boost/noncopyable.hpp>
#include <boost/asio/bind_executor.hpp>
namespace IOEvent
{
namespace Http
{
template <typename _Timer, typename CompletionExecutor>
class Timer final
	:boost::noncopyable
{
	using self_type = Timer;
public:
	using timer_type = _Timer;
	using clock_type = typename timer_type::clock_type;
	using duration_type = typename timer_type::duration;
	using time_point = typename timer_type::time_point;
public:
	template <typename TimePointOrDuration>
	explicit Timer(const CompletionExecutor & executor, const TimePointOrDuration duration_or_time)
		:completion_executor_{ executor },
		timer_(completion_executor_.get_inner_executor(), duration_or_time)
	{}
	timer_type& stream() { return timer_; }

	template <typename Function>
	void async_wait(Function &&func)
	{
		timer_.async_wait(boost::asio::bind_executor(completion_executor_, std::forward<Function>(func)));
	}
	
	auto wait()->decltype(boost::system::error_code{})
	{
		auto error = boost::system::error_code{};
		timer_.wait(error);
		return error;
	}

	auto cancel()->decltype(boost::system::error_code{})
	{
		auto error = boost::system::error_code{};
		timer_.cancel(error);
		return error;
	}
		
private:
	CompletionExecutor const& completion_executor_;
	timer_type timer_;
};
}
}





#endif // !_IOEVENT_HTTP_TIMER_H