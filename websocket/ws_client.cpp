#include <memory>
#include <string>
#include <utility>
#include <cassert>
#include "logging.h"
#include "ws_connection.h"
#include "ws_client.h"
#include "http_request.h"
#include "http_response.h"
#include "http_server.h"
#include "SHA1.h"
#include "base64.h"

namespace tinynet
{


WsClient::WsClient(EventLoop *event_loop, std::string name)
:_name(name), 
 _tcp_client(event_loop, name),
 _handshake_done(false)
{
    _tcp_client.set_newconn_cb(std::bind(&WsClient::handle_new_connection, this, std::placeholders::_1));
    _tcp_client.set_onmessage_cb(std::bind(&WsClient::handle_message, this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3));
    _tcp_client.set_disconnected_cb(std::bind(&WsClient::handle_disconnected, this, std::placeholders::_1));
    _tcp_client.set_write_complete_cb(std::bind(&WsClient::handle_write_complete, this, std::placeholders::_1));

}

bool WsClient::connect(std::string server_ip, int server_port)
{
    return _tcp_client.connect(server_ip, server_port);
}

void WsClient::handle_write_complete(TcpConnPtr &conn)
{
    if (nullptr != _ws_conn)
    {
        if (_write_complete_cb)
        {
            _write_complete_cb(ws_conn);
        }
    }
    else
    {
        LOG(ERROR) << "_ws_conn is NULL, something wrong in write_cb" << std::endl;
    }
}

void WsClient::handle_new_connection(TcpConnPtr &conn)
{
    LOG(INNER_DEBUG) << _name <<": new conn " << conn->get_name() << std::endl;
    // conn->set_context(HttpResponse());

    if(nullptr != _ws_conn)
    {
        LOG(ERROR) << "something wrong, _ws_client is not NULL in new_conn_cb" << std::endl;
        _ws_conn.reset();
    }
    _ws_conn = std::make_shared<WebSocketConnection>(conn, conn->get_name());
    handshake_req(conn);
}

void WsClient::handle_disconnected(TcpConnPtr &conn)
{
    LOG(DEBUG) << _name << " disconnected:" << conn->get_name() << std::endl;
    if (nullptr != _ws_conn)
    {
        assert((_ws_conn._tcp_conn)->get_fd() == conn->get_fd());
        if (_disconnected_cb)
        {
            _disconnected_cb(_ws_conn);
        }
        LOG(DEBUG) << "release _ws_conn" << std::endl
        _ws_conn.reset();
    }
    else
    {
        LOG(ERROR) << "_ws_conn is NULL, something wrong!" << std::endl;
    }

    _handshake_done = false;
}

void WsClient::handshake_req(TcpConnPtr &conn)
{
    std::string http_req = "GET / HTTP/1.1\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n";
    if (nullptr != conn)
    {
        LOG(INFO) << _name << " initiates a handshake request" << std::endl;
        conn->write_data(http_req.data(), http_req.size());
    }
}

void WsClient::handle_http_resp(const HttpRequest &request, HttpResponse &response)
{
    // FIXME: should check response
}

void WsClient::handle_message(TcpConnPtr &conn, const uint8_t *data, size_t size)
{

    LOG(DEBUG) << "_handshake_done:" << _handshake_done << std::endl; 
    if(!_handshake_done)
    {
        std::string raw_resp(reinterpret_cast<const char*>(data), size);
        LOG(DEBUG) << " http raw response: " << raw_resp << std::endl;
        
        // FIXME: should process http response
        // std::any &context = conn->get_context();
        // HttpResponse &request = std::any_cast<HttpResponse&>(context);
        // bool http_complete = HttpClient::process_http_response(conn, raw_resp, request, 
        //     std::bind(&WebSocketServer::handle_http_resp, this, std::placeholders::_1, std::placeholders::_2));

        if (_newconn_cb)
        {
            _newconn_cb(_ws_conn);
        }
       _handshake_done = true;
    }
    else
    {
        if(_on_message_cb)
        {
            _ws_conn->handle_recv_data(data, size, _on_message_cb);
        }
        else
        {
            LOG(WARNING) << _name << " The message processing callback function is not configured, and the disconnection is performed." << std::endl;
            // conn->disable_conn();
        }
    }

}

}  // namespace tinynet

