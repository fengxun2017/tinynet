#ifndef _TINYNET_TCP_ACCEPTOR_H_
#define _TINYNET_TCP_ACCEPTOR_H_
#include <functional>
#include <iostream>
#include <string>
#include "io_socket.h"
#include "io_channel.h"
#include "tcp_connection.h"
#include "event_loop.h"
namespace tinynet
{

class TcpAcceptor {
public:
    using NewConnCb = std::function<void(std::shared_ptr<TcpConnection> &conn)>;
    TcpAcceptor(EventLoop *event_loop, const std::string& ip, int port, std::string name);
    ~TcpAcceptor();
    void set_newconn_cb(NewConnCb newconn_cb) {_newconn_cb = newconn_cb;}
    bool start();
    void stop();

private:
    void accept_connection(void);

    std::string _name;
    EventLoop *_event_loop = nullptr;
    IoSocket _acceptor_socket;
    std::string _ip;
    int _port;
    IoChannel _channel;
    NewConnCb   _newconn_cb = nullptr;
};

}  // namespace tinynet

#endif  // _TINYNET_CONN_ACCEPTOR_H_
