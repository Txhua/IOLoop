#include "Regex.h"

namespace IOEvent
{
namespace Http
{
Regex::Regex(flag_type flag)
	:flag_(flag)
{
}

bool Regex::match(const std::string& regex, const std::string& str)
{
	const regex_type e{ regex, flag_ };
	return std::regex_match(str, e);	
}

bool Regex::match(const std::string& regex, const std::string& str, std::smatch& result)
{
	const regex_type e{ regex, flag_ };
	return std::regex_match(str, result, e);
}
}
}


