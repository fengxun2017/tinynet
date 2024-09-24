#ifndef _TINYNET_TCP_SERVER_H_
#define _TINYNET_TCP_SERVER_H_
#include <string>
#include <functional>
#include <unordered_map>
#include "event_loop.h"
#include "tcp_connection.h"
#include "tcp_acceptor.h"

namespace tinynet
{

class TcpServer {
public:
    using NewConnCb = std::function<void(TcpConnection&)>;
    using DisconnectedCb = std::function<void(const TcpConnection&)>;
    using OnMessageCb = std::function<void(const TcpConnection&)>;
    using WriteCompleteCb = std::function<void(const TcpConnection&)>;

    TcpServer(EventLoop *event_loop, const std::string& ip, int port, std::string name);
    ~TcpServer(void);

    void set_newconn_cb(NewConnCb newconn_cb) {
        _newconn_cb = newconn_cb;
    }

    void set_disconnected_cb(DisconnectedCb disconnected_cb) {
        _disconnected_cb = disconnected_cb;
    }

    void set_onmessage_cb(OnMessageCb on_message_cb) {
        _on_message_cb = on_message_cb;
    }

    void set_write_complete_cb(WriteCompleteCb write_complete_cb) {
        _write_complete_cb = write_complete_cb;
    }

    bool start(void);
    void stop(void);

private:
    void handle_new_connection(TcpConnPtr conn);

    void handle_disconnected(TcpConnPtr conn);

    void handle_message(TcpConnPtr conn, const uint8_t *data, size_t size);

    void handle_write_complete(TcpConnPtr conn);
    std::string _name;
    TcpAcceptor _acceptor;
    std::unordered_map<int, TcpConnPtr> _connections;
    NewConnCb _newconn_cb = nullptr;
    DisconnectedCb _disconnected_cb = nullptr;
    OnMessageCb _on_message_cb = nullptr;
    WriteCompleteCb _write_complete_cb = nullptr;
    EventLoop *_event_loop = nullptr;
};

}  // namespace tinynet

#endif  // _TINYNET_TCP_SERVER_H_
