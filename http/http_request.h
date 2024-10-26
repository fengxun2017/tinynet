#ifndef _TINYNET_HTTP_REQ_H_
#define _TINYNET_HTTP_REQ_H_

#include <string>
#include <unordered_map>
namespace tinynet
{

class HttpRequest {
public:
    enum HttpMethod { GET = 0, POST, UNKNOWN_METHOD };
    enum HttpVersion {HTTP10 = 0, HTTP11, UNKNOWN_VER};
    enum HttpRecvState {};
    HttpRequest() : _method(UNKNOWN_METHOD) {}
    ~HttpRequest() {}

    bool parse(const std::string& raw_request);
    HttpVersion get_version(void);
    std::string get_header(const std::string &key);
    HttpMethod get_method(void) const  {return _method;}
    std::string get_url(void) const {return _URL;}
    std::string get_body(void) const {return _body;}

    std::string dump_heads(void) const;
private:
    static const std::string CRLF;
    static const std::string CRLFCRLF;

    HttpMethod _method;
    std::string _URL;
    std::unordered_map<std::string, std::string> _headers;
    std::string _version;
    std::string _body;
    void parse_headers(const std::string& headers_str);
    HttpMethod convert_to_method(std::string &method_str);

};

} // namespace tinynet

#endif // _TINYNET_HTTP_REQ_H_