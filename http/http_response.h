#ifndef _TINYNET_HTTP_RESP_H_
#define _TINYNET_HTTP_RESP_H_
#include <string>
#include <unordered_map>

namespace tinynet
{

class HttpResponse {
public:
    HttpResponse() : _version("HTTP/1.1"), _status_code(200), _status_message("OK"), _need_close(false) {}

    void set_status(int code, const std::string &message)
    {
        _status_code = code;
        _status_message = message;
    }

    void add_header(const std::string &key, const std::string &value)
    {
        _headers[key] = value;
    }

    void set_body(const std::string &body_content);

    void set_version(const std::string &version) {_version = version;}

    void set_need_close(bool need_close) {_need_close = need_close;}
    bool get_need_close(void) {return _need_close;}
    std::string to_string() const;

private:
    std::string _version;
    int _status_code;
    std::string _status_message;
    std::unordered_map<std::string, std::string> _headers;
    std::string _body_info;
    uint32_t content_len = 0;
    bool _need_close;
};

} // namespace tiynet
#endif // _TINYNET_HTTP_RESP_H_
