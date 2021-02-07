#include "Url.h"
#include <assert.h>

namespace IOEvent
{

Url::Url(const std::string &url)
{
    if(!url.empty())
    {
        parse(url);
    }  
}


void Url::parse(const std::string &url)
{
    // http://localhost:8080/root?date=5
    std::string tmp = url;
    ltrim(tmp);
    auto p = std::string::npos;

    p = tmp.find("://");
    if (p != std::string::npos) 
    {
        scheme_ = tmp.substr(0, p);
        tmp = tmp.substr(p + 3);
    }

    p = tmp.find('/');
    if (p != std::string::npos) 
    {
        host_ = tmp.substr(0, p);

        tmp = tmp.substr(p);

        p = tmp.find('?');
        if (p != std::string::npos) 
        {
            path_ = tmp.substr(0, p);
            query_ = tmp.substr(p + 1);
        } 
        else 
        {
            path_ = tmp;
        }
    } 
    else 
    {
        path_ = "";
        p = tmp.find('?');
        if (p != std::string::npos) 
        {
            host_ = tmp.substr(0, p);
            query_ = tmp.substr(p + 1);
        } 
        else 
        {
            host_ = tmp;
        }
    }

    if (!host_.empty()) 
    {
        // Check if there's a port.
        p = host_.find_last_of(':');
        if (p != std::string::npos) 
        {
            // For IPv6: [::1]:8080
            std::size_t bracket = host_.find_last_of(']');
            if (bracket == std::string::npos || p > bracket) 
            {
                port_ = host_.substr(p + 1);
                host_ = host_.substr(0, p);
            }
        }
    }
}

void Url::appendPath(const std::string &str)
{
    // path_ = /root
    // str = /data 
    if (str.empty() || str == "/") 
    {
        return;
    }

    if (path_.empty() || path_ == "/") 
    {
        path_.clear();
        if (str.front() != '/') 
        {
            path_.push_back('/');
        }
    } 
    else if (path_.back() == '/' && str.front() == '/') 
    {
        path_.pop_back();
    } 
    else if (path_.back() != '/' && str.front() != '/') 
    {
        path_.push_back('/');
    }
    path_.append(str); 
}

void Url::appendQuery(const std::string &key, const std::string &value)
{
    if(query_.empty())
    {
        query_ += "&";
    }
    query_ += key + "=" + value;
}

std::string &Url::ltrim(std::string &str, const std::string &chars) 
{
    str.erase(0, str.find_first_not_of(chars));
    return str;
}


}