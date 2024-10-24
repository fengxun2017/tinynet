
#include "http_server.h"
#include "logging.h"
namespace tinynet
{

void HttpServer::new_conn_cb(tinynet::TcpConnPtr &conn)
{
    LOG(DEBUG) << _name <<": new conn " << conn->get_name() << std::endl;
}

void HttpServer::disconnected_cb(tinynet::TcpConnPtr &conn)
{
    LOG(DEBUG) << "echo_server: " << conn->get_name() << " disconnected" << std::endl;
}

void HttpServer::on_message_cb(tinynet::TcpConnPtr &conn, const uint8_t *data, size_t size)
{
    std::string raw_request(static_cast<const char*>(data), size);
    LOG(DEBUG) << raw_request << std::endl;

    HttpRequest request;
    if (!request.parse(raw_request))
    {
        LOG(ERROR) << _name << "Failed to parse HTTP request" << std::endl;
        return;
    }

    HttpResponse response;
    if (request.method == HttpRequest::GET)
    {
        response.set_body("Hello, world!");
    } 
    else
    {
        response.set_status(501, "Not Implemented");
        response.set_body("Unsupported method");
    }

    std::string raw_response = response.to_string();
    conn->write_data(static_cast<const void*>(raw_response.data()), raw_response.size());

    std::string connection = request.get_header("Connection");
    bool close = false;
    if ((request.get_version() == HttpRequest::HTTP10 && connection != "Keep-Alive")
        || connection == "close"
        || (request.get_version() == HttpRequest::UNKNOWN))
    {
        close = true;
    }
    if (close)
    {
        conn->disable_conn();
    }
}

HttpServer::HttpServer(EventLoop *event_loop, const std::string& ip, int port, std::string name)
:_name(name), 
 _tcp_server(event_loop, ip, port, name)
{
    _tcp_server.set_newconn_cb(std::bind(&HttpServer::new_conn_cb, this, std::placeholders::_1));
    _tcp_server.set_onmessage_cb(std::bind(&HttpServer::on_message_cb, this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3));
    _tcp_server.set_disconnected_cb(std::bind(&HttpServer::disconnected_cb, this, std::placeholders::_1));
}

void HttpServer::set_onrequest_cb(HttpOnRequestCb cb)
{
    _onrequest_cb = cb;
}

void HttpServer::start(void)
{
    _tcp_server.start();
}

} // namespace tinynet