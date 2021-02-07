#ifndef _IOEVENT_HTTP_CHAIN_ROUTER_H
#define _IOEVENT_HTTP_CHAIN_ROUTER_H

#include <Router.hpp>

namespace IOEvent
{
namespace Http
{
template <typename HttpSession>
class ChainRouter
	: public Router<HttpSession>
{
	using self_type = ChainRouter;
	using base_type = Router<HttpSession>;
	class chain_node;
	friend chain_node;
public:
	using session_type = typename base_type::session_type;
    using storage_type = typename base_type::storage_type;
    using resource_map_type = typename base_type::resource_map_type;
    using method_map_type = typename base_type::method_map_type;
    using resource_regex_type = typename base_type::resource_regex_type;
    using method_type = typename base_type::method_type;
    using regex_type = typename base_type::regex_type;
    using regex_flag_type = typename base_type::regex_flag_type;
    using request_type = typename base_type::request_type;
    using chain_node_type = chain_node;
public:
    ChainRouter(regex_flag_type regex_flags) noexcept
        :base_type{ resource_map_, method_map_ },
		regex_flags_{ regex_flags }
	{}
	
    chain_node route(resource_regex_type const& path_to_resource)&
    {
        save_to_res_regex(path_to_resource);
        return chain_node{ *this };
    }

    void use(resource_regex_type const& path_to_resource, base_type const& other)
    {
        base_type::use(path_to_resource, other);
    }

    void use(base_type const& other)
    {
        base_type::use("", other);
    }

    regex_flag_type regex_flags() const
    {
        return regex_flags_;
    }
private:
	class chain_node
	{
        using router_type = self_type;
        using node_type = chain_node;
	public:
		explicit chain_node(router_type& router) noexcept
            :router_(route)
		{}
        template<typename... OnRequest>
        auto get(OnRequest&&... on_request)
            -> decltype (storage_type(std::declval<OnRequest>()...), std::declval<node_type&>())
        {
            router_.add_resource_cb(router_.tmp_res_regex_, method_type::get, storage_type{ std::forward<OnRequest>(on_request)... });
            return *this;
        }

        template<typename... OnRequest>
        auto post(OnRequest&&... on_request)
            -> decltype (storage_type(std::declval<OnRequest>()...), std::declval<node_type&>())
        {
            router_.add_resource_cb(router_.tmp_res_regex_, method_type::post, storage_type{ std::forward<OnRequest>(on_request)... });
            return *this;
        }

        template<typename... OnRequest>
        auto put(OnRequest&&... on_request)
            -> decltype (storage_type(std::declval<OnRequest>()...), std::declval<node_type&>())
        {
            router_.add_resource_cb(router_.tmp_res_regex_, method_type::put, storage_type{ std::forward<OnRequest>(on_request)... });
            return *this;
        }

        template<typename... OnRequest>
        auto head(OnRequest&&... on_request)
            -> decltype (storage_type(std::declval<OnRequest>()...), std::declval<node_type&>())
        {
            router_.add_resource_cb(router_.tmp_res_regex_, method_type::head, storage_type{ std::forward<OnRequest>(on_request)... });
            return *this;
        }

        template<typename... OnRequest>
        auto delete_(OnRequest&&... on_request)
            -> decltype (storage_type(std::declval<OnRequest>()...), std::declval<node_type&>())
        {
            router_.add_resource_cb(router_.tmp_res_regex_, method_type::delete_, storage_type{ std::forward<OnRequest>(on_request)... });
            return *this;
        }

        template<typename... OnRequest>
        auto options(OnRequest&&... on_request)
            -> decltype (storage_type(std::declval<OnRequest>()...), std::declval<node_type&>())
        {
            router_.add_resource_cb(router_.tmp_res_regex_, method_type::options, storage_type{ std::forward<OnRequest>(on_request)... });
            return *this;
        }

        template<typename... OnRequest>
        auto connect(OnRequest&&... on_request)
            -> decltype (storage_type(std::declval<OnRequest>()...), std::declval<node_type&>())
        {
            router_.add_resource_cb(router_.tmp_res_regex_, method_type::connect, storage_type{ std::forward<OnRequest>(on_request)... });
            return *this;
        }

        template<typename... OnRequest>
        auto trace(OnRequest&&... on_request)
            -> decltype (storage_type(std::declval<OnRequest>()...), std::declval<node_type&>())
        {
            router_.add_resource_cb(router_.tmp_res_regex_, method_type::trace, storage_type{ std::forward<OnRequest>(on_request)... });
            return *this;
        }

	private:
        router_type& router_;
	};

private:
    inline void save_to_res_regex(resource_regex_type const& path_to_resource)
    {
        tmp_res_regex_ = path_to_resource;
    }
	
private:  
    regex_flag_type regex_flags_;
	//临时变量，用于存储用于执行路线链任务的正则表达式
    resource_regex_type tmp_res_regex_;
    std::shared_ptr<resource_map_type> resource_map_;
    std::shared_ptr<method_map_type> method_map_;

};
}
}







#endif // !_IOEVENT_HTTP_CHAIN_ROUTER_H