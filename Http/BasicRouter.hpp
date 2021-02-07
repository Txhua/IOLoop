#ifndef _IOEVENT_HTTP_BASIC_ROUTER_H
#define _IOEVENT_HTTP_BASIC_ROUTER_H

#include "Router.hpp"

namespace IOEvent
{
namespace Http
{
template <typename HttpSession>
class BasicRouter
	: public Router<HttpSession>
{
	using base_type = Router<HttpSession>;
	using self_type = BasicRouter;
	using base_type::add_resource_cb;
	using base_type::add_resource_without_method;
public:
	using session_type = typename base_type::session_type;
	using storage_type = typename base_type::storage_type;
	using resource_map_type = typename base_type::resource_map_type;
	using method_map_type = typename base_type::method_map_type;
	using method_type = typename base_type::method_type;
	using resource_regex_type = typename base_type::resource_regex_type;
	using regex_type = typename base_type::regex_type;
	using regex_flag_type = typename base_type::regex_flag_type;
	using request_type = typename base_type::request_type;
public:
	BasicRouter(regex_flag_type regex_flag) noexcept
		:base_type(resource_map_, method_map_),
		regex_flags_(regex_flag)
	{
		
	}
	template <typename ...OnRequest>
	auto get(resource_regex_type const &path_to_resource, OnRequest &&...on_request) & 
		->decltype(void(storage_type(std::declval<OnRequest>()...)))
	{
		add_resource_cb(path_to_resource, method_type::get, storage_type{ std::forward<OnRequest>(on_request)... });
	}

	template <typename ...OnRequest>
	auto post(resource_regex_type const &path_to_resource, OnRequest &&...on_request) &
		->decltype(void(storage_type(std::declval<OnRequest>()...)))
	{
		add_resource_cb(path_to_resource, method_type::post, storage_type{ std::forward<OnRequest>(on_request)... });
	}

	template <typename ...OnRequest>
	auto put(resource_regex_type const &path_to_resource, OnRequest &&...on_request) &
		->decltype(void(storage_type(std::declval<OnRequest>()...)))
	{
		add_resource_cb(path_to_resource, method_type::put, storage_type{ std::forward<OnRequest>(on_request)... });
	}

	template<typename ...OnRequest>
	auto head(resource_regex_type const& path_to_resource, OnRequest&&... on_request) &
		-> decltype (void(storage_type(std::declval<OnRequest>()...)))
	{
		add_resource_cb(path_to_resource, method_type::head, storage_type{ std::forward<OnRequest>(on_request)... });
	}

	template <typename ...OnRequest>
	auto delete_ (resource_regex_type const& path_to_resource, OnRequest&&... on_request) &
		-> decltype (void(storage_type(std::declval<OnRequest>()...)))
	{
		add_resource_cb(path_to_resource, method_type::delete_, storage_type{ std::forward<OnRequest>(on_request)... });
	}

	template <typename ...OnRequest>
	auto connect(resource_regex_type const& path_to_resource, OnRequest&&... on_request) &
		->decltype(void(storage_type{std::declval<OnRequest>()...}))
	{
		add_resource_cb(path_to_resource, method_type::connect, storage_type{ std::forward<OnRequest>(on_request)... });
	}

	template <typename ...OnRequest>
	auto options(resource_regex_type const& path_to_resource, OnRequest&&... on_request) &
		->decltype(void(storage_type{ std::declval<OnRequest>()... }))
	{
		add_resource_cb(path_to_resource, method_type::options, storage_type{ std::forward<OnRequest>(on_request)... });
	}

	template <typename ...OnRequest>
	auto trace(resource_regex_type const& path_to_resource, OnRequest&&... on_request) &
		->decltype(void(storage_type{ std::declval<OnRequest>()... }))
	{
		add_resource_cb(path_to_resource, method_type::trace, storage_type{ std::forward<OnRequest>(on_request)... });
	}

	void use(resource_regex_type const& path_to_resource, self_type const& other) { base_type::use(path_to_resource, other); }
	
	void use(self_type const& other) { base_type::use("", other); }

	regex_flag_type regex_flags() const { return regex_flags_; }
	
private:
	regex_flag_type regex_flags_;
	std::shared_ptr<resource_map_type> resource_map_;
	std::shared_ptr<method_map_type> method_map_;
};
	
}
}




#endif // !_IOEVENT_HTTP_BASIC_ROUTER_H