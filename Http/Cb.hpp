#ifndef _IOEVENT_HTTP_CB_H
#define _IOEVENT_HTTP_CB_H

#include <iterator>
#include <tuple>
#include <functional>
#include "Traits.hpp"

#define HTTP_DECLARE_STORAGE_TEMPLATE \
	 typename HttpSession, \
	 template <typename Signature> class Entry, \
	 template <typename Key, typename ... Args> class Container
#define HTTP_DECLARE_STORAGE_ATTRIBUTES \
	 HttpSession, Entry, Container

namespace IOEvent
{
namespace Http
{

struct ForEach
{
	template<std::size_t Index, typename Begin, typename End, typename... Elements>
	static void invoke(const std::tuple<Elements...>& tpl, const Begin& begin, const End& end)
	{
		const auto& value = std::get<Index>(tpl);
		if constexpr (Index + 1 == std::tuple_size<std::tuple<Elements...>>::value)
			end(value);
		else {
			begin(value);
			ForEach::invoke<Index + 1, Begin, End, Elements...>(tpl, begin, end);
		}
	}
};


class executor
{
protected:

	template<typename Request, typename SessionFlesh, typename _Storage>
	void execute(Request &request, SessionFlesh &flesh, _Storage &storage);

};

template <typename Request, typename SessionFlesh, typename _Storage>
void executor::execute(Request &request, SessionFlesh &flesh, _Storage &storage)
{
	storage.begin_exec(request, flesh)();
}


template
<
	typename HttpSession,
	template <typename Signature> class Entry,
	template <typename Element, typename ...Args> class Container
>
struct ConstIterator
{
	template<typename, template <typename> class, template <typename, typename ...> class>
	friend class Storage;

	using self_type = ConstIterator;
	using session_type = HttpSession;
	using session_flesh = typename session_type::flesh_type;
	using session_context = typename session_type::context_type;
	using request_type = typename session_type::request_type;
	using entry_type = Entry<void(request_type, session_context, self_type)>;
	using container_type = Container<entry_type>;
	using container_iterator = typename container_type::const_iterator;
	using size_type = typename container_type::size_type;

	// iterator_traits
	using iterator_category = std::input_iterator_tag; // µü´úÆ÷Àà±ð
	using value_type = typename container_iterator::value_type;
	using difference_type = typename container_iterator::difference_type;
	using pointer = typename container_iterator::pointer;
	using reference = typename container_iterator::reference;

public:
	ConstIterator(const container_type& container, request_type& request, session_flesh& flesh);
	void skip_target();
	self_type& operator++();
	self_type& operator++(int);
	void operator()();
	inline size_type pos() { return pos_; }
private:
	size_type pos_;
	container_iterator cont_begin_iter_;
	container_iterator cont_end_iter_;
	request_type& request_;
	session_flesh& session_flesh_;
	std::string current_target_;
};



template <HTTP_DECLARE_STORAGE_TEMPLATE>
ConstIterator<HTTP_DECLARE_STORAGE_ATTRIBUTES>::ConstIterator(const container_type& container, request_type& request, session_flesh& flesh)
	:pos_{ 0 },
	cont_begin_iter_{ container.begin() },
	cont_end_iter_{ container.end() },
	request_{ request },
	session_flesh_{ flesh },
	current_target_{ request_.target().to_string() }
{
	if(container.size() > 1)
	{
		skip_target();
	}
}

template <HTTP_DECLARE_STORAGE_TEMPLATE>
void ConstIterator<HTTP_DECLARE_STORAGE_ATTRIBUTES>::skip_target()
{
	std::size_t pos = current_target_.find("/", 1);
	if(pos != std::string::npos)
	{
		auto next_target = current_target_.substr(0, pos);
		current_target_ = current_target_.substr(pos);
		request_.target(next_target);
	}
	else
	{
		request_.target(current_target_);
	}
}

template <HTTP_DECLARE_STORAGE_TEMPLATE>
typename ConstIterator<HTTP_DECLARE_STORAGE_ATTRIBUTES>::self_type& ConstIterator<HTTP_DECLARE_STORAGE_ATTRIBUTES>::operator++()
{
	cont_begin_iter_++;
	pos_++;
	if(cont_begin_iter_ == cont_end_iter_)
	{
		cont_begin_iter_--;
		pos_--;
	}
	skip_target();
	return *this;
}

template <HTTP_DECLARE_STORAGE_TEMPLATE>
typename ConstIterator<HTTP_DECLARE_STORAGE_ATTRIBUTES>::self_type& ConstIterator<HTTP_DECLARE_STORAGE_ATTRIBUTES>::operator++(int)
{
	self_type _tmp{ *this };
	++(*this);
	return _tmp;
}

template <HTTP_DECLARE_STORAGE_TEMPLATE>
void ConstIterator<HTTP_DECLARE_STORAGE_ATTRIBUTES>::operator()()
{
	session_context ctx{ session_flesh_ };
	(*cont_begin_iter_)(request_, std::move(ctx), *this);
}

template
<
	typename HttpSession,
	template <typename Signature> class Entry,
	template <typename Element, typename ...Args> class Container
>
class Storage
{
	using self_type = Storage;
public:
	using session_type = HttpSession;
	using session_flesh = typename session_type::flesh_type;
	using session_context = typename session_type::context_type;
	using request_type = typename session_type::request_type;
	using iterator_type = ConstIterator<HTTP_DECLARE_STORAGE_ATTRIBUTES>;
	using entry_type = Entry<void(request_type, session_context, iterator_type)>;
	using container_type = Container<entry_type>;
	friend class executor;
public:
	Storage() = default;
	template
	<
		typename Function,
		typename ...Fn,
		typename = typename std::enable_if
		<
			!std::is_same<typename std::decay<Function>::type, self_type>::value
			&& TryInvokeConjunction
				<
					sizeof ...(Fn),
					void(request_type, session_context, iterator_type),
					void(request_type, session_context), Function, Fn...
				>::value
		>::type
	>
	Storage(Function&& function, Fn && ...fn);
private:
	template <typename ...OnRequest>
	container_type prepare(OnRequest && ...on_request);
	iterator_type begin_exec(request_type &request, session_flesh &flesh);
private:
	container_type container_;
};

template <HTTP_DECLARE_STORAGE_TEMPLATE>
template <typename Function, typename ... Fn, typename>
Storage<HTTP_DECLARE_STORAGE_ATTRIBUTES>::Storage(Function&& function, Fn&&... fn)
	:container_{ prepare(std::forward<Function>(function), std::forward<Fn>(fn)...)}
{
}

template <HTTP_DECLARE_STORAGE_TEMPLATE>
template <typename ... OnRequest>
typename Storage<HTTP_DECLARE_STORAGE_ATTRIBUTES>::container_type Storage<HTTP_DECLARE_STORAGE_ATTRIBUTES>::prepare(OnRequest&&... on_request)
{
	container_type container;
	const auto &tuple_cb = std::make_tuple(std::forward<OnRequest>(on_request)...);
	static_assert(std::tuple_size<typename std::decay<decltype (tuple_cb) >::type>::value != 0, "Oops...! tuple is empty.");
	ForEach::invoke<0>(tuple_cb, [&container](const auto& value)
	{
		container.push_back(entry_type(std::bind<void>(value, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
	},
	[&container](const auto &value)
	{
		container.push_back(entry_type(std::bind<void>(value, std::placeholders::_1, std::placeholders::_2)));
	});
	return container;
}

template <HTTP_DECLARE_STORAGE_TEMPLATE>
typename Storage<HTTP_DECLARE_STORAGE_ATTRIBUTES>::iterator_type Storage<HTTP_DECLARE_STORAGE_ATTRIBUTES>::begin_exec(
	request_type& request, session_flesh& flesh)
{
	return iterator_type(container_, request, flesh);
}
}	
}












#endif // !_IOEVENT_HTTP_CB_H