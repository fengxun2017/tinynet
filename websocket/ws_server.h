#ifndef _TINYNET_WEBSOCKET_SERVER_H_
#define _TINYNET_WEBSOCKET_SERVER_H_

#include <string>
#include <functional>
#include <tuple>
#include "tcp_server.h"
#include "event_loop.h"
#include "http_request.h"
#include "http_response.h"
#include "ws_connection.h"
namespace tinynet
{

class WebSocketServer {
public:
    using WSSrvNewConnCb = std::function<void(WsConnPtr&)>;
    using WSSrvDisconnectedCb = std::function<void(WsConnPtr&)>;
    using WSSrvOnMessageCb = std::function<void(WsConnPtr&, const uint8_t *data, size_t size)>;
    using WSSrvWriteCompleteCb = std::function<void(WsConnPtr&)>;

    WebSocketServer(EventLoop *event_loop, const std::string& ip, int port, std::string name);
    ~WebSocketServer(){}
    void start();

    void set_newconn_cb(WSSrvNewConnCb newconn_cb) {
        _newconn_cb = newconn_cb;
    }

    void set_disconnected_cb(WSSrvDisconnectedCb disconnected_cb) {
        _disconnected_cb = disconnected_cb;
    }

    void set_onmessage_cb(WSSrvOnMessageCb on_message_cb) {
        _on_message_cb = on_message_cb;
    }

    void set_write_complete_cb(WSSrvWriteCompleteCb write_complete_cb) {
        _write_complete_cb = write_complete_cb;
    }


private:
    static const std::string MAGIC_KEY;
    void handle_new_connection(TcpConnPtr &conn);

    void handle_disconnected(TcpConnPtr &conn);

    void handle_write_complete(TcpConnPtr &conn);

    void handle_message(TcpConnPtr &conn, const uint8_t *data, size_t size);

    void handle_http_request(const HttpRequest &request, HttpResponse &response);

    WSSrvNewConnCb _newconn_cb = nullptr;
    WSSrvDisconnectedCb _disconnected_cb = nullptr;
    WSSrvOnMessageCb _on_message_cb = nullptr;
    WSSrvWriteCompleteCb _write_complete_cb = nullptr;
    std::string _name;
    TcpServer _tcp_server;
    bool _handshake_done;
    std::unordered_map<int, std::tuple<bool, WsConnPtr>> _ws_clients;
};

} // tinynet


#endif // _TINYNET_WEBSOCKET_SERVER_H_