#ifndef _IOEVENT_ROUTER_H
#define _IOEVENT_ROUTER_H

#include "Traits.hpp"
#include "ShareMutex.hpp"
#include <memory>
#include <assert.h>



namespace IOEvent
{
namespace Http
{

template <typename HttpSession>
class Router
{
	using self_type = Router;
protected:
	using session_type = HttpSession;
	using mutex_type = ShareMutex::mutex_type;
	using storage_type = typename session_type::storage_type;
	using resource_map_type = typename session_type::resource_map_type; // std::unordered_map<std::string, Http::Storage<self_type, Entry, Container>>
	using method_map_type = typename session_type::method_map_type;     // std::map<method_type, std::unordered_map>
	using resource_regex_type = typename session_type::resource_regex_type;
	using resource_type = typename session_type::resource_type;
	using method_type = typename session_type::method_type;
	using regex_type = typename session_type::regex_type;
	using request_type = typename session_type::request_type;
	using regex_flag_type = typename session_type::regex_flag_type;
	
	static_assert(detail::conjunction
					<
						HasStorageType<session_type, void>,
						HasResourceMapType<session_type, void>
					>::value, "Session type is incorrect!");
public:
	explicit Router(std::shared_ptr<resource_map_type>& resource_map, std::shared_ptr<method_map_type>& method_map) noexcept;
	void add_resource_cb(const resource_regex_type& path_to_resource, const method_type& method, storage_type&& storage);
	void add_resource_without_method(const resource_regex_type& path_to_resource, storage_type&& storage);
	const std::shared_ptr<resource_map_type>& resource_map() const { return resource_map_; }
	const std::shared_ptr<method_map_type>& method_map() const { return method_map_; }
	mutex_type& mutex() const;
	void use(resource_regex_type const& path_to_resource, self_type const& other);
private:
	resource_regex_type concat(const resource_regex_type&, const resource_regex_type&);
private:
	mutable mutex_type mutex_;
	std::shared_ptr<resource_map_type>& resource_map_;
	std::shared_ptr<method_map_type>& method_map_;
};

template <typename HttpSession>
Router<HttpSession>::Router(std::shared_ptr<resource_map_type>& resource_map,std::shared_ptr<method_map_type>& method_map) noexcept
		:resource_map_{ resource_map },
		method_map_{ method_map }
{
}

template <typename HttpSession>
void Router<HttpSession>::add_resource_cb(const resource_regex_type& path_to_resource, const method_type& method, storage_type&& storage)
{
	assert(!path_to_resource.empty());
	HTTP_SHARED_MUTEX_ENTER_TO_WRITE(mutex_);
	if(!method_map_)
	{
		method_map_ = std::make_shared<method_map_type>();
	}
	auto &resource_map = method_map_->insert({ method, resource_map_type() }).first->second;
	auto res = resource_map.emplace(path_to_resource, std::move(storage));
	if(!res.second)
	{
		res.first->second = std::move(storage);
	}
}

template <typename HttpSession>
void Router<HttpSession>::add_resource_without_method(const resource_regex_type& path_to_resource, storage_type&& storage)
{
	assert(!path_to_resource.empty());
	HTTP_SHARED_MUTEX_ENTER_TO_WRITE(mutex_);
	if(!resource_map_)
	{
		resource_map_ = std::shared_ptr<resource_map_type>();
	}
	auto res = resource_map_->emplace(path_to_resource, std::move(storage));
	if (!res.second)
	{
		res.first->second = std::move(storage);
	}
}

template <typename HttpSession>
typename Router<HttpSession>::mutex_type& Router<HttpSession>::mutex() const
{
	return static_cast<mutex_type&>(mutex_);
}

template <typename HttpSession>
void Router<HttpSession>::use(resource_regex_type const& path_to_resource, self_type const& other)
{
	HTTP_SHARED_MUTEX_ENTER_TO_READ(other.mutex_);
	if(other.resource_map_)
	{
		for(const auto &value : *other.resource_map_)
		{
			auto storage = value.second;
			add_resource_without_method(concat(path_to_resource, value.first), std::move(storage));
		}
	}

	if(other.method_map_)
	{
		for (const auto& value_m : *other.method_map_) 
		{
			auto method = value_m.first;
			const auto& resource_map = value_m.second;

			for (const auto& value_r : resource_map) 
			{
				auto storage = value_r.second;
				add_resource_cb(concat(path_to_resource, value_r.first), method, std::move(storage));
			}
		}
	}
}

template <typename HttpSession>
typename Router<HttpSession>::resource_regex_type Router<HttpSession>::concat(const resource_regex_type& resource1, const resource_regex_type& resource2)
{
	resource_regex_type result;
	if (resource1.back() == '$' && resource1.front() == '^' && resource2.back() == '$' && resource2.front() == '^')
	{
		result = resource1.substr(0, resource1.size() - 1) + resource2.substr(1);
	}
	else if (resource1.back() == '$' && resource1.front() == '^')
	{
		result = resource1.substr(0, resource1.size() - 1) + resource2 + "$";
	}
	else if (resource2.back() == '$' && resource2.front() == '^')
	{
		result = "^" + resource1 + resource2.substr(1);
	}
	else
	{
		result = resource1 + resource2;
	}

	return result;
}
} // Http
} // IOEvent











#endif // !_IOEVENT_ROUTER_H