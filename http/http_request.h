#ifndef _TINYNET_HTTP_REQ_H_
#define _TINYNET_HTTP_REQ_H_

#include <string>
#include <unordered_map>

class HttpRequest {
public:
    enum HttpMethod { GET, POST, UNKNOWN };

    HttpRequest() : method(UNKNOWN) {}
    ~HttpRequest() {}

    bool parse(const std::string& raw_request);

private:
    static const std::string CRLF;
    static const std::string CRLFCRLF;

    HttpMethod _method;
    std::string _URL;
    std::unordered_map<std::string, std::string> _headers;
    std::string _version;

    void parse_headers(const std::string& headers_str);
    HttpMethod convert_to_method(std::string &method_str);

};

#endif // _TINYNET_HTTP_REQ_H_