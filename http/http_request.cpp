
#include <sstream>
#include "logging.h"
#include "http_request.h"

namespace tinynet
{

const std::string HttpRequest::CRLF = "\r\n";
const std::string HttpRequest::CRLFCRLF = "\r\n\r\n";

HttpRequest::HttpMethod HttpRequest::convert_to_method(std::string &method_str)
{
    HttpMethod method = UNKNOWN_METHOD;

    if (method_str == "GET")
    {
        method = GET;
    }
    else if (method_str == "POST")
    {
        method = POST;
    }
    else
    {
         method = UNKNOWN_METHOD;
    }

    return method;
}

bool HttpRequest::parse(const std::string &raw_request) {
    std::string::size_type method_end = raw_request.find(' ');
    if (method_end == std::string::npos)
    {
        LOG(ERROR) << "http request error: can not find method" << std::endl;
        return false;
    }
    std::string method_str = std::move(raw_request.substr(0, method_end));
    _method = convert_to_method(method_str);

    std::string::size_type url_start = method_end + 1;
    std::string::size_type url_end = raw_request.find(' ', url_start);
    if (url_end == std::string::npos)
    {
        LOG(ERROR) << "http request error: can not find URL" << std::endl;
        return false;
    }
    _URL = std::move(raw_request.substr(url_start, url_end - url_start));

    std::string::size_type version_start = url_end + 1;
    std::string::size_type version_end = raw_request.find(CRLF, version_start);
    if (version_end == std::string::npos)
    {
        LOG(ERROR) << "http request error: can not find version" << std::endl;
        return false;
    }
    _version = std::move(raw_request.substr(version_start, version_end - version_start));

    std::string::size_type headers_start = raw_request.find(CRLF) + 2;
    std::string::size_type headers_end = raw_request.find(CRLFCRLF);
    if (headers_start == std::string::npos || headers_end == std::string::npos)
    {
        LOG(ERROR) << "http request error: header format error" << std::endl;
        return false;
    }
    /* Make each header keep |name|:|sp|val|CRLF structure */
    headers_end += 2;
    std::string headers_str = std::move(raw_request.substr(headers_start, headers_end - headers_start));
    parse_headers(headers_str);

    std::string::size_type body_start = headers_end + 2;
    _body = std::move(raw_request.substr(body_start, raw_request.size()-body_start));
    return true;
}

void HttpRequest::parse_headers(const std::string& headers_str) {
    std::string::size_type start = 0;
    std::string::size_type end = headers_str.find(CRLF);

    while (end != std::string::npos) {
        std::string line = std::move(headers_str.substr(start, end - start));
        std::string::size_type delimiter = line.find(':');
        if (delimiter != std::string::npos) {
            std::string key = line.substr(0, delimiter);
            /* There is also a space after the delimiter */
            std::string val = line.substr(delimiter + 2);
            // LOG(DEBUG) << "key:" << key << " val:" << val << std::endl;
            _headers.emplace(std::make_pair(std::move(key), std::move(val)));
        }
        start = end + 2;
        end = headers_str.find(CRLF, start);
    }
}

std::string HttpRequest::get_header(const std::string &key)
{
    std::string result;
    auto item = _headers.find(key);
    if (item != _headers.end())
    {
      result = item->second;
    }
    return result;
}

std::string HttpRequest::dump_heads(void) const
{
    std::ostringstream oss;
    oss << "{";
    for (const auto &pair : _headers) {
        oss << " " << pair.first << ":" << pair.second << ",";
    }
    return oss.str();
}


HttpRequest::HttpVersion HttpRequest::get_version(void)
{
    HttpVersion ver = UNKNOWN_VER;
    
    if ("HTTP/1.1" == _version)
    {
        ver = HTTP11;
    }
    else if ("HTTP/1.0" == _version)
    {
        ver = HTTP10;
    }

    return ver;
}

} // tinynet