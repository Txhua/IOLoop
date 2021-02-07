#ifndef _IOEVENT_STRAND_STREAM_H
#define _IOEVENT_STRAND_STREAM_H

#include <boost/asio/strand.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/system_timer.hpp>

namespace IOEvent
{
struct StrandStream
	: boost::asio::strand<boost::asio::system_timer::executor_type>
{
	using asio_type = boost::asio::strand<boost::asio::system_timer::executor_type>;
	StrandStream(const boost::asio::system_timer::executor_type& executor)
		:asio_type(executor)
	{}
};
}






#endif // !_IOEVENT_STRAND_STREAM_H