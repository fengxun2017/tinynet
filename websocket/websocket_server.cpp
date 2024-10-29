#include <string>
#include <utility>
#include "logging.h"
#include "websocket_server.h"
#include "http_request.h"
#include "http_response.h"
#include "http_server.h"
#include "SHA1.h"
#include "base64.h"

namespace tinynet
{

const std::string WebSocketServer::MAGIC_KEY = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

WebSocketServer::WebSocketServer(EventLoop *event_loop, const std::string& ip, int port, std::string name)
:_name(name), 
 _tcp_server(event_loop, ip, port, name),
 _handshake_done(false)
{
    _tcp_server.set_newconn_cb(std::bind(&WebSocketServer::handle_new_connection, this, std::placeholders::_1));
    _tcp_server.set_onmessage_cb(std::bind(&WebSocketServer::handle_message, this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3));
    _tcp_server.set_disconnected_cb(std::bind(&WebSocketServer::handle_disconnected, this, std::placeholders::_1));
}


void WebSocketServer::start(void)
{
    _tcp_server.start();
}

void WebSocketServer::handle_new_connection(TcpConnPtr &conn)
{
    conn->set_context(HttpRequest());
    LOG(DEBUG) << _name <<": new conn " << conn->get_name() << std::endl;
}

void WebSocketServer::handle_disconnected(TcpConnPtr &conn)
{
    _handshake_done = false;
    LOG(DEBUG) << _name << " disconnected:" << conn->get_name() << std::endl;
}

void WebSocketServer::handle_http_request(const HttpRequest &request, HttpResponse &response)
{
    if (request.get_method() == tinynet::HttpRequest::GET)
    {
        std::string sec_key = request.get_header("Sec-Websocket-Key");
        if(sec_key.empty())
        {
            response.set_status(400, "Bad Request");
        }
        else
        {
            uint8_t hash[20];
            sec_key.append(MAGIC_KEY);
            secure::SHA1::compute_sha1(sec_key.data(), sec_key.size(), hash, sizeof(hash));
            char base64[32] = {0};
            to_base64(hash, 20, base64, sizeof(base64));
            
            response.set_status(101, "Switching Protocols");
            response.add_header("Upgrade", "websocket");
            response.add_header("Connection", "Upgrade");
        }
    }
    else
    {
        response.set_status(501, "Not Implemented");
        response.set_body("Unsupported method");
        response.set_need_close(true);
    }
}

void WebSocketServer::handle_message(TcpConnPtr &conn, const uint8_t *data, size_t size)
{
    bool close = false;
    bool bad_request =false;

    std::string raw_request(reinterpret_cast<const char*>(data), size);
    LOG(DEBUG) << " http raw request: " << raw_request << std::endl;

    std::any &context = conn->get_context();
    HttpRequest &request = std::any_cast<HttpRequest&>(context);
 
    HttpServer::process_http_request(conn, raw_request, request, _onrequest_cb);

}

}  // namespace tinynet

