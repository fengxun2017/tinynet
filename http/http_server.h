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
    using HttpOnRequestCb = std::function<void (const HttpRequest&,HttpResponse&)>;

    HttpServer(EventLoop *event_loop, const std::string& ip, int port, std::string name);
    ~HttpServer(){}
    void set_onrequest_cb(HttpOnRequestCb cb);
    void start();

private:
    void HttpServer::on_message_cb(tinynet::TcpConnPtr &conn, const uint8_t *data, size_t size);
    void HttpServer::disconnected_cb(tinynet::TcpConnPtr &conn);
    void HttpServer::new_conn_cb(tinynet::TcpConnPtr &conn);
    std::string _name;
    TcpServer _tcp_server;
    HttpOnRequestCb _onrequest_cb = nullptr;
};

} // tinynet


#endif // _TINYNET_HTTP_SERVER_H_