#include <string>
#include <utility>
#include "logging.h"
#include "websocket_server.h"
#include "http_request.h"
#include "http_response.h"
#include "http_server.h"

namespace tinynet
{

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



void WebSocketServer::handle_message(TcpConnPtr &conn, const uint8_t *data, size_t size)
{
    bool close = false;
    bool bad_request =false;

    std::string raw_request(reinterpret_cast<const char*>(data), size);
    LOG(DEBUG) << " http raw request: " << raw_request << std::endl;

    std::any &context = conn->get_context();
    HttpRequest &request = std::any_cast<HttpRequest&>(context);
 
    
}

}  // namespace tinynet

