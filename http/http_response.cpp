#include "http_response.h"

namespace tinynet
{

void HttpResponse::set_body(const std::string &body_content)
{
    _body_info = body_content;
    _headers["Content-Length"] = std::to_string(_body_info.size());
}

std::string HttpResponse::to_string() const 
{
    std::string response = _version + " " + std::to_string(_status_code) + " " + _status_message + "\r\n";
    for (const auto& header : _headers) {
        response += header.first + ": " + header.second + "\r\n";
    }
    response += "\r\n" + _body_info;

    return response;
}

} // tinynet
