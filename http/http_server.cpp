
#include<any>
#include "http_server.h"
#include "http_request.h"
#include "logging.h"
#include "event_loop.h"

namespace tinynet
{

void HttpServer::new_conn_cb(TcpConnPtr &conn)
{
    conn->set_context(HttpRequest());
    LOG(DEBUG) << _name <<": new conn " << conn->get_name() << std::endl;
}

void HttpServer::disconnected_cb(TcpConnPtr &conn)
{
    LOG(DEBUG) << _name << " disconnected:" << conn->get_name() << std::endl;
}

bool HttpServer::process_http_request(TcpConnPtr &conn, std::string &raw_request, HttpRequest &request, HttpServer::HttpOnRequestCb onrequest_cb)
{
    bool close = false;
    bool parse_success;
    HttpResponse response;
    bool recv_complete = false;

    parse_success = request.parse(raw_request);
    if (!parse_success)
    {
        LOG(ERROR) << ": Failed to parse HTTP request" << std::endl;
        response.set_status(400, "Bad Request");
        close = true;
    }
    else
    {
        if (request.recv_complete())
        {
            recv_complete = true;
            // LOG(DEBUG) << "RECV COMPLETE" << std::endl;
            if (request.get_version() > HttpRequest::HTTP11)
            {
                LOG(INFO) << ": HTTP Version Not Supported" << std::endl; 
                response.set_status(505, "HTTP Version Not Supported");
                close = true;
            }
            else
            {
                if (onrequest_cb)
                {
                    onrequest_cb(request, response);
                    close = response.get_need_close();
                }
                else
                {
                    LOG(INFO) << "The HTTP request handler is not registered" << std::endl;
                    response.set_status(501, "Not Implemented");
                    response.set_body("Unsupported method");
                    close = true;
                }
            }
            std::string connection = request.get_header("Connection");
            // LOG(DEBUG) << "Connection:" << connection << std::endl;
            if (request.get_version() == HttpRequest::HTTP10 && connection != "Keep-Alive")
            {
                LOG(DEBUG) << ": http1.0 and not set keep-alive" << std::endl;
                close = true;
            }
            if (connection == "close")
            {
                LOG(DEBUG) << ": http client need to close" << std::endl;
                close = true;
            }
        }
    }

    if(!parse_success || request.recv_complete())
    {
        std::string raw_response = response.to_string();
        conn->write_data(static_cast<const void*>(raw_response.data()), raw_response.size());
        request.reset();
    }

    if (close)
    {
        conn->disable_conn();
    }

    return recv_complete;
}

void HttpServer::on_message_cb(TcpConnPtr &conn, const uint8_t *data, size_t size)
{
    bool close = false;
    bool bad_request =false;

    std::string raw_request(reinterpret_cast<const char*>(data), size);
    LOG(DEBUG) << " http raw request: " << raw_request << std::endl;

    std::any &context = conn->get_context();
    HttpRequest &request = std::any_cast<HttpRequest&>(context);
    // HttpRequest request;

    process_http_request(conn, raw_request, request, _onrequest_cb);
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