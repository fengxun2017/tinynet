#ifndef _TINYNET_TCP_SERVER_H_
#define _TINYNET_TCP_SERVER_H_
#include <string>
#include <functional>
#include <unordered_map>
#include "tcp_connection.h"
#include "tcp_acceptor.h"

namespace tinynet
{

class TcpServer {
public:
    using NewConnCb = std::function<void(const TcpConnection&)>;
    using DisconnectedCb = std::function<void(const TcpConnection&)>;
    using OnMessageCb = std::function<void(const TcpConnection&)>;
    using WriteCompleteCb = std::function<void(const TcpConnection&)>;

    TcpServer(const std::string& ip, int port);
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
    void handle_new_connection(std::shared_ptr<TcpConnection>& conn);

    void handle_disconnected(const TcpConnection& conn);

    void handle_message(const TcpConnection& conn);

    void handle_write_complete(const TcpConnection& conn);
    bool conn_check_and_remove(TcpConnection& conn);

    TcpAcceptor _acceptor;
    std::unordered_map<int, std::shared_ptr<TcpConnection>> _connections;
    NewConnCb _newconn_cb = nullptr;
    DisconnectedCb _disconnected_cb = nullptr;
    OnMessageCb _on_message_cb = nullptr;
    WriteCompleteCb _write_complete_cb = nullptr;
};

}  // namespace tinynet

#endif  // _TINYNET_TCP_SERVER_H_