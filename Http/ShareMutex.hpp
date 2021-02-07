#ifndef _IOEVENT_LOCKABLE_H
#define _IOEVENT_LOCKABLE_H

#include <boost/core/ignore_unused.hpp>
#include <shared_mutex>
#include <mutex>

#define HTTP_SHARED_MUTEX_ENTER_TO_READ(shared_mutex) \
	auto const& dummy = ShareMutex::enter_to_read(shared_mutex); \
	boost::ignore_unused(dummy);

#define HTTP_SHARED_MUTEX_ENTER_TO_WRITE(shared_mutex) \
	auto const& dummy = ShareMutex::enter_to_write(shared_mutex); \
	boost::ignore_unused(dummy);

namespace IOEvent
{
struct ShareMutex
{
	using mutex_type = std::shared_mutex;
	using shared_lock_type = std::shared_lock<mutex_type>;
	using unique_lock_type = std::unique_lock<mutex_type>;
	static auto enter_to_read(mutex_type &mutex) -> decltype(shared_lock_type(mutex))
	{
		return shared_lock_type(mutex);
	}

	static auto enter_to_write(mutex_type& mutex) -> decltype(unique_lock_type(mutex))
	{
		return unique_lock_type(mutex);
	}	
};
}






#endif // !_IOEVENT_LOCKABLE_H