#include <memory>
#include <string>
#include <utility>
#include "logging.h"
#include "ws_connection.h"
#include "ws_server.h"
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
    _tcp_server.set_write_complete_cb(std::bind(&WebSocketServer::handle_write_complete, this, std::placeholders::_1));

}

void WebSocketServer::start(void)
{
    _tcp_server.start();
}

void WebSocketServer::handle_write_complete(TcpConnPtr &conn)
{
    auto  item = _ws_clients.find(conn->get_fd());
    if (item != _ws_clients.end())
    {
        auto &ws_conn = std::get<1>(item->second);
        if (_write_complete_cb)
        {
            _write_complete_cb(ws_conn);
        }
    }
}

void WebSocketServer::handle_new_connection(TcpConnPtr &conn)
{
    LOG(DEBUG) << _name <<": new conn " << conn->get_name() << std::endl;
    // bool websocket_handshake_done = false;
    conn->set_context(HttpRequest());
    // conn->set_context2(websocket_handshake_done);

#ifdef TINYNET_DEBUG
    auto item = _ws_clients.find(conn->get_fd());
    if (item != _ws_clients.end())
    {
        LOG(ERROR) << "The same file descriptor already exists! something wrong" << std::endl; 
    }
    else
#endif
    {
        // LOG(INFO) << "The connection from " << new_conn->get_client_ip() 
        //     << ":" << new_conn->get_client_port() << " is established" 
        //     << std::endl;
        WsConnPtr ws_conn = std::make_shared<WebSocketConnection>(conn, conn->get_name());
        _ws_clients.emplace(std::make_pair(conn->get_fd(), std::tuple<bool, WsConnPtr>(false, ws_conn)));
        if (_newconn_cb)
        {
            _newconn_cb(ws_conn);
        }
    }
}

void WebSocketServer::handle_disconnected(TcpConnPtr &conn)
{
    LOG(DEBUG) << _name << " disconnected:" << conn->get_name() << std::endl;
    auto item = _ws_clients.find(conn->get_fd());
    if (item != _ws_clients.end())
    {
        auto &ws_conn = std::get<1>(item->second);
        if (_disconnected_cb) {
            _disconnected_cb(ws_conn);
        }
        // The release should be at the end
        (void)_ws_clients.erase(item);
    }
    else
    {
        LOG(ERROR) << "error in WebSocketServer::handle_disconnected, "
                << "the client was not found from the client collection " << std::endl;
    }

    _handshake_done = false;
}

void WebSocketServer::handle_http_request(const HttpRequest &request, HttpResponse &response)
{
    if (request.get_method() == tinynet::HttpRequest::GET)
    {
        std::string sec_key = request.get_header("Sec-WebSocket-Key");
        if(sec_key.empty())
        {
            LOG(DEBUG) << "Sec-WebSocket-Key must be present!" << std::endl;
            response.set_status(400, "Bad Request");
        }
        else
        {
            uint8_t hash[20];
            sec_key.append(MAGIC_KEY);
            secure::SHA1::compute_sha1((uint8_t *)(sec_key.data()), sec_key.size(), hash, sizeof(hash));
            char base64[32] = {0};
            secure::to_base64(hash, 20, base64, sizeof(base64));
            response.add_header("Sec-Websocket-Accept", base64);
            response.add_header("Upgrade", "websocket");
            response.add_header("Connection", "Upgrade");
            response.set_status(101, "Switching Protocols");
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

    auto client = _ws_clients.find(conn->get_fd());
    if (client != _ws_clients.end())
    {
        bool &websocket_handshake_done = std::get<0>(client->second);
        // std::any &context2 = conn->get_context2();
        // bool &websocket_handshake_done = std::any_cast<bool&>(context2);
        LOG(DEBUG) << "websocket_handshake_done:" << websocket_handshake_done << std::endl; 
        if(!websocket_handshake_done)
        {
            // FIXME: maybe some user data in "data, size"
            std::string raw_request(reinterpret_cast<const char*>(data), size);
            LOG(DEBUG) << " http raw request: " << raw_request << std::endl;
            std::any &context = conn->get_context();
            HttpRequest &request = std::any_cast<HttpRequest&>(context);

            bool http_complete = HttpServer::process_http_request(conn, raw_request, request, 
                std::bind(&WebSocketServer::handle_http_request, this, std::placeholders::_1, std::placeholders::_2));
            
            if (http_complete)
            {
                websocket_handshake_done = true;
            }
        }
        else
        {
            if(_on_message_cb)
            {
                auto ws_conn = std::get<1>(client->second);
                ws_conn->handle_recv_data(data, size, _on_message_cb);
            }
            else
            {
                LOG(INFO) << _name << " The message processing callback function is not configured, and the disconnection is performed." << std::endl;
                conn->disable_conn();
            }
        }
    }
    else
    {
        LOG(ERROR) << "something wrong, client not in _ws_clients" << std::endl;
    }
}

}  // namespace tinynet

