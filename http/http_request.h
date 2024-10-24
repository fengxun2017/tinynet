#ifndef _TINYNET_HTTP_REQ_H_
#define _TINYNET_HTTP_REQ_H_

#include <string>
#include <unordered_map>

class HttpRequest {
public:
    enum HttpMethod { GET = 0, POST, UNKNOWN };
    enum HttpVersion {HTTP10 = 0, HTTP11, UNKNOWN};

    HttpRequest() : _method(UNKNOWN) {}
    ~HttpRequest() {}

    bool parse(const std::string& raw_request);
    HttpVersion get_version(void);
    const std::string get_header(const std::string &key);
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