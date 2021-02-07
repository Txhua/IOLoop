#ifndef _IOEVENT_HTTP_REQUEST_PROCESSOR_H
#define _IOEVENT_HTTP_REQUEST_PROCESSOR_H

#include <memory>

namespace IOEvent
{
namespace Http
{
template <typename HttpSession>
class RequestProcessor : private HttpSession::executor_type
{
	using session_type = HttpSession;
	using self_type = RequestProcessor;
public:
	using session_flesh = typename session_type::flesh_type;
	using storage_type = typename session_type::storage_type;
	using resource_map_type = typename session_type::resource_map_type; // std::unordered_map<std::string, Http::Storage<self_type, Entry, Container>>
	using method_map_type = typename session_type::method_map_type;     // std::map<method_type, std::unordered_map>
	using resource_type = typename session_type::resource_type;
	using method_type = typename session_type::method_type;
	using regex_type = typename session_type::regex_type;
	using request_type = typename session_type::request_type;
public:
	RequestProcessor(std::shared_ptr<resource_map_type> const& resource_map, 
					 std::shared_ptr<method_map_type> const& method_map,
					 typename regex_type::flag_type flags)
		:resource_map_{ resource_map },
		method_map_{ method_map },
		regex_{flags}
	{}

	void provide(request_type& request, session_flesh& flesh);
private:
	regex_type regex_;
	std::shared_ptr<resource_map_type> const& resource_map_;
	std::shared_ptr<method_map_type> const& method_map_;	
};

template <typename HttpSession>
void RequestProcessor<HttpSession>::provide(request_type& request, session_flesh& flesh)
{
	auto target = request.target();
	method_type method = request.method();
	if(method_map_)
	{
		auto method_pos = static_cast<method_map_type&>(*method_map_).find(method);
		if(method_pos != method_map_->cend())
		{
			auto& resource_map = method_pos->second;
			for (auto __it_value = resource_map.cbegin(); __it_value != resource_map.cend(); ++__it_value)
			{
				if(regex_.match(__it_value->first, target.to_string()))
				{
					auto& storage = const_cast<storage_type&>(__it_value->second);
					this->execute(request, flesh, storage);
				}
			}
		}
	}
}
	
}
}








#endif // !_IOEVENT_HTTP_REQUEST_PROCESSOR_H