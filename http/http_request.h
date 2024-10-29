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
    ~HttpRequest() {}
    HttpRequest(void);

    bool parse(const std::string& raw_request);
    HttpVersion get_version(void);
    std::string get_header(const std::string &key) const;
    HttpMethod get_method(void) const  {return _method;}
    std::string get_url(void) const {return _URL;}
    std::string get_body(void) const {return _body;}
    std::string dump_heads(void) const;
    bool recv_complete(void) {return _state == RECV_COMPLETE;}
    void reset(void);

private:
    static const std::string CRLF;
    static const std::string CRLFCRLF;
    enum HttpRecvState {WAIT_REQUEST_LINE, WAIT_HEADERS,WAIT_BODY, RECV_COMPLETE};

    bool parse_request_line(const std::string& raw_request);
    bool parse_headers(const std::string& raw_request);
    bool parse_body(const std::string& raw_request);

    HttpMethod convert_to_method(std::string &method_str);

    HttpMethod _method;
    std::string _URL;
    std::unordered_map<std::string, std::string> _headers;
    std::string _version;
    std::string _body;
    size_t _body_len = 0;
    std::string _request_buffer;
    HttpRecvState _state;

};

} // namespace tinynet

#endif // _TINYNET_HTTP_REQ_H_