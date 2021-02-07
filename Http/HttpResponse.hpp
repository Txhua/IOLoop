#ifndef _IOEVEHT_HTTP_RESPONSE_H
#define _IOEVEHT_HTTP_RESPONSE_H

#include <boost/beast/http/message.hpp>
#include <boost/beast/version.hpp>

namespace IOEvent
{
namespace Http
{
template <typename ResponseBody, typename RequestBody>
class HttpResponse
{
public:
	HttpResponse() {};
	auto make_200(boost::beast::http::request<RequestBody> &request, typename ResponseBody::value_type body)
	{
		boost::beast::http::response<ResponseBody> response{ boost::beast::http::status::ok, request.version() };
		response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
		response.set(boost::beast::http::field::content_type, "text/html");
		response.keep_alive(request.keep_alive());
		response.body() = body;
		response.prepare_payload();
		return response;
	}
};
}
}




#endif // !_IOEVEHT_HTTP_RESPONSE_H