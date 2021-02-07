#ifndef _IOEVENT_URL_H
#define _IOEVENT_URL_H

#include <boost/noncopyable.hpp>
#include <string>
#include <regex>

namespace IOEvent
{

class Url final 
{
public:
    Url() = default;
    explicit Url(const std::string &url);
    Url(Url&&) = default;
    Url &operator=(Url&&) = default;
    const std::string &scheme() const { return scheme_; }
    const std::string &host() const { return host_; }
    const std::string &port() const { return port_; }
    const std::string &path() const { return path_; }
    const std::string &query() const { return query_; }
    void setPort(const std::string &port) { port_ = port; }
    void appendPath(const std::string &path);
    void appendQuery(const std::string &key, const std::string &value);
private:
    void parse(const std::string &url);
    std::string &ltrim(std::string &str, const std::string &chars = "\t ");
private:
    std::string scheme_;
    std::string host_;
    std::string port_;
    std::string path_;
    std::string query_;
};

class UrlRegex final 
    : boost::noncopyable
{
public:
    explicit UrlRegex(const std::string &url)
        :url_(url)
    {}

    std::regex operator()()const
    {
        std::regex::flag_type flags = std::regex::ECMAScript | std::regex::icase;
        return std::regex(url_, flags);
    }

private:
    std::string url_;
};

using Regex1 = UrlRegex;

}





#endif // ! _IOEVENT_URL_H