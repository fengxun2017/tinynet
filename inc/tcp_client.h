#ifndef _TINYNET_TCP_CONNECTOR_H_
#define _TINYNET_TCP_CONNECTOR_H_

#include <string>
#include "event_loop.h"
#include "tcp_connection.h"
#include "tcp_connector.h"

namespace tinynet
{

class TcpClient {
public:
    using TcpClientNewConnCb = std::function<void(TcpConnPtr&)>;
    using TcpClientDisconnectedCb = std::function<void(TcpConnPtr&)>;
    using TcpClientOnMessageCb = std::function<void(TcpConnPtr&, const uint8_t *data, size_t size)>;
    using TcpClientWriteCompleteCb = std::function<void(TcpConnPtr&)>;

    TcpClient(EventLoop *event_loop, std::string &client_name);

    ~TcpClient();

    bool connect(std::string& server_ip, int server_port);

    void set_newconn_cb(TcpClientNewConnCb newconn_cb) {
        _newconn_cb = newconn_cb;
    }

    void set_disconnected_cb(TcpClientDisconnectedCb disconnected_cb) {
        _disconnected_cb = disconnected_cb;
    }

    void set_onmessage_cb(TcpClientOnMessageCb on_message_cb) {
        _on_message_cb = on_message_cb;
    }

    void set_write_complete_cb(TcpClientWriteCompleteCb write_complete_cb) {
        _write_complete_cb = write_complete_cb;
    }
private:

    void handle_new_connection(TcpConnPtr conn);

    void handle_disconnected(TcpConnPtr conn);

    void handle_message(TcpConnPtr conn, const uint8_t *data, size_t size);

    void handle_write_complete(TcpConnPtr conn);

    std::string _name;
    std::string _remote_ip;
    int _remote_port;
    EventLoop *_event_loop;
    TcpConnector _connector;
    TcpConnPtr _conn= nullptr;
    TcpClientNewConnCb _newconn_cb = nullptr;
    TcpClientDisconnectedCb _disconnected_cb = nullptr;
    TcpClientOnMessageCb _on_message_cb = nullptr;
    TcpClientWriteCompleteCb _write_complete_cb = nullptr;
};

}


#endif // _TINYNET_TCP_CONNECTOR_H_