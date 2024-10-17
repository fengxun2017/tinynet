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
    using TcpSrvNewConnCb = std::function<void(TcpConnPtr&)>;
    using TcpSrvDisconnectedCb = std::function<void(TcpConnPtr&)>;
    using TcpSrvOnMessageCb = std::function<void(TcpConnPtr&, const uint8_t *data, size_t size)>;
    using TcpSrvWriteCompleteCb = std::function<void(TcpConnPtr&)>;

    TcpServer(EventLoop *event_loop, const std::string& ip, int port, std::string name);
    ~TcpServer(void);

    void set_newconn_cb(TcpSrvNewConnCb newconn_cb) {
        _newconn_cb = newconn_cb;
    }

    void set_disconnected_cb(TcpSrvDisconnectedCb disconnected_cb) {
        _disconnected_cb = disconnected_cb;
    }

    void set_onmessage_cb(TcpSrvOnMessageCb on_message_cb) {
        _on_message_cb = on_message_cb;
    }

    void set_write_complete_cb(TcpSrvWriteCompleteCb write_complete_cb) {
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
    TcpSrvNewConnCb _newconn_cb = nullptr;
    TcpSrvDisconnectedCb _disconnected_cb = nullptr;
    TcpSrvOnMessageCb _on_message_cb = nullptr;
    TcpSrvWriteCompleteCb _write_complete_cb = nullptr;
    EventLoop *_event_loop = nullptr;
};

}  // namespace tinynet

#endif  // _TINYNET_TCP_SERVER_H_
