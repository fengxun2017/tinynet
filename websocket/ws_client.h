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
#include "tcp_client.h"
namespace tinynet
{

class WsClient {
public:
    using WsClientNewConnCb = std::function<void(WsConnPtr&)>;
    using WsClientDisconnectedCb = std::function<void(WsConnPtr&)>;
    using WsClientOnMessageCb = std::function<void(WsConnPtr&, const uint8_t *data, size_t size)>;
    using WsClientWriteCompleteCb = std::function<void(WsConnPtr&)>;

    WsClient(EventLoop *event_loop, std::string name);
    ~WsClient(){}
    bool connect(std::string server_ip, int server_port);

    
    void set_newconn_cb(WsClientNewConnCb newconn_cb) {
        _newconn_cb = newconn_cb;
    }

    void set_disconnected_cb(WsClientDisconnectedCb disconnected_cb) {
        _disconnected_cb = disconnected_cb;
    }

    void set_onmessage_cb(WsClientOnMessageCb on_message_cb) {
        _on_message_cb = on_message_cb;
    }

    void set_write_complete_cb(WsClientWriteCompleteCb write_complete_cb) {
        _write_complete_cb = write_complete_cb;
    }

private:
    static const std::string MAGIC_KEY;
    void handshake_req(TcpConnPtr &conn);
    void handle_new_connection(TcpConnPtr &conn);

    void handle_disconnected(TcpConnPtr &conn);

    void handle_write_complete(TcpConnPtr &conn);

    void handle_message(TcpConnPtr &conn, const uint8_t *data, size_t size);

    void handle_http_resp(const HttpRequest &request, HttpResponse &response);

    WsClientNewConnCb _newconn_cb = nullptr;
    WsClientDisconnectedCb _disconnected_cb = nullptr;
    WsClientOnMessageCb _on_message_cb = nullptr;
    WsClientWriteCompleteCb _write_complete_cb = nullptr;
    std::string _name;
    TcpClient _tcp_client;
    bool _handshake_done;
    WsConnPtr _ws_conn = nullptr;
};

} // tinynet


#endif // _TINYNET_WEBSOCKET_SERVER_H_