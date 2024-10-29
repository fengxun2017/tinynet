#ifndef _TINYNET_HTTP_SERVER_H_
#define _TINYNET_HTTP_SERVER_H_

#include <string>
#include <functional>
#include "tcp_server.h"
#include "event_loop.h"
#include "http_request.h"
#include "http_response.h"

namespace tinynet
{

class HttpServer {
public:
    using HttpOnRequestCb = std::function<void (const HttpRequest&, HttpResponse&)>;

    HttpServer(EventLoop *event_loop, const std::string& ip, int port, std::string name);
    ~HttpServer(){}
    void set_onrequest_cb(HttpOnRequestCb cb);
    void start();
    static bool process_http_request(TcpConnPtr &conn, std::string &raw_request, HttpRequest &request, HttpServer::HttpOnRequestCb onrequest_cb);

private:
    void on_message_cb(TcpConnPtr &conn, const uint8_t *data, size_t size);
    void disconnected_cb(TcpConnPtr &conn);
    void new_conn_cb(TcpConnPtr &conn);
    std::string _name;
    TcpServer _tcp_server;
    HttpOnRequestCb _onrequest_cb = nullptr;
};

} // tinynet


#endif // _TINYNET_HTTP_SERVER_H_