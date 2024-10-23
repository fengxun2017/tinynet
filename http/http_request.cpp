
#include "http_request.h"

namespace tinynet
{

const std::string CRLF = "\r\n";
const std::string CRLFCRLF = "\r\n\r\n";

HttpMethod HttpRequest::convert_to_method(std::string &method_str)
{
    HttpMethod method = UNKNOWN;

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
         method = UNKNOWN;
    }

    return method;
}

bool HttpRequest::parse(const std::string &raw_request) {
    std::string::size_type method_end = raw_request.find(' ');
    if (method_end == std::string::npos)
    {
        return false;
    }
    std::string method_str = std::move(raw_request.substr(0, method_end));
    _method = convert_to_method(method_str);

    std::string::size_type url_start = method_end + 1;
    std::string::size_type url_end = raw_request.find(' ', url_start);
    if (url_end == std::string::npos)
    {
        return false;
    }
    std::string _URL = std::move(raw_request.substr(url_start, url_end - url_start));

    std::string::size_type version_start = url_end + 1;
    std::string::size_type version_end = raw_request.find(CRLF, version_start);
    if (version_end == std::string::npos)
    {
        return false;
    }
    std::string _version = std::move(raw_request.substr(version_start, version_end - version_start));

    std::string::size_type headers_start = raw_request.find(CRLF) + 2;
    std::string::size_type headers_end = raw_request.find(CRLFCRLF);
    if (headers_start == std::string::npos || headers_end == std::string::npos)
    {
        return false;
    }
    /* Make each header keep |name|:|sp|val|CRLF structure */
    headers_end -= 2;

    std::string headers_str = std::move(raw_request.substr(headers_start, headers_end - headers_start));
    parse_headers(headers_str);

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

            _headers.emplace(std::make_pair(std::move(key), std::move(val)));
        }
        start = end + 2;
        end = headers_str.find("\r\n", start);
    }
}

} // tinynet