#ifndef _IOEVENT_HTTP_QUEUE_H
#define _IOEVENT_HTTP_QUEUE_H

#include <vector>
#include <memory>
#include <assert.h>

namespace IOEvent
{
namespace Http
{

template <typename Flesh>
class Queue
{
	const static size_t limit = 16;

	struct work
	{
		virtual ~work() = default;
		virtual void operator() () = 0;
	};
	Flesh& impl_;
	std::vector<std::unique_ptr<work>> items_;
public:
	explicit Queue(Flesh &flesh)
		:impl_(flesh)
	{
		static_assert(limit > 0, "queue limit must be positive");
		items_.reserve(limit);
	}

	bool on_write()
	{
		assert(!items_.empty());
		auto const was_full = is_full();
		items_.erase(items_.begin());
		if(!items_.empty())
		{
			(*items_.front())();
		}
		return was_full;
	}


	bool is_full() const
	{
		return items_.size() >= limit;
	}

	template <typename Response>
	void operator() (Response& response);	
};

template <typename Flesh>
template <typename Response>
void Queue<Flesh>::operator()(Response& response)
{
	using response_type = typename std::decay<Response>::type;
	struct work_impl : work
	{
		Flesh& impl_;
		response_type response_;
		work_impl(Flesh &impl, response_type &&response)
			:impl_(impl),
			response_(response)
		{
			
		}

		void operator()() override
		{
			impl_.do_write(response_);
		}
	};
	items_.push_back(std::make_unique<work_impl>(impl_, std::move(response)));
	if(items_.size() == 1)
	{
		(*items_.front())();
	}
}
}
}







#endif // !_IOEVENT_HTTP_QUEUE_H