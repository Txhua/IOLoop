#ifndef _IOEVENT_HTTP_REGEX_H
#define _IOEVENT_HTTP_REGEX_H

#include <boost/noncopyable.hpp>
#include <regex>

namespace IOEvent
{
namespace Http
{
class Regex
	: public boost::noncopyable
{
	using self_type = Regex;
public:
	using char_type = char;
	using traits_type = std::regex_traits<char_type>;
	using regex_type = std::basic_regex<char_type, traits_type>;
	using flag_type = typename regex_type::flag_type;
public:
	bool match(const std::string& regex, const std::string& str);
	bool match(const std::string& regex, const std::string& str, std::smatch& result);
public:
	Regex(flag_type);
private:
	flag_type flag_;
};
}
}









#endif // !_IOEVENT_HTTP_REGEX_H