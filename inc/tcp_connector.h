#ifndef _TINYNET_TCP_CONNECTOR_H_
#define _TINYNET_TCP_CONNECTOR_H_

#include <memory>
#include <string>
#include "io_socket.h"
#include "io_channel.h"
#include "event_loop.h"
#include "tcp_connection.h"
namespace tinynet
{

class TcpConnector {
    using TcpConnectorNewConnCb = std::function<void(TcpConnPtr)>;
    using TcpConnectorDisConnCb = std::function<void(TcpConnPtr)>;

public:
    TcpConnector(EventLoop *event_loop, std::string name);

    ~TcpConnector();

    bool connect(const std::string &server_ip, int server_port);

    void set_newconn_cb(TcpConnectorNewConnCb newconn_cb) {
        _newconn_cb = newconn_cb;
    }

    void set_disconnected_cb(TcpConnectorDisConnCb disconnected_cb) {
        _disconnected_cb = disconnected_cb;
    }

private:
    void handle_write_complete(void);
    void connecting(void);

    std::string _name;
    std::unique_ptr<IoSocket> _connector_socket;
    EventLoop *_event_loop;
    std::unique_ptr<IoChannel> _channel;
    std::string _server_ip;
    int _server_port;
    TcpConnectorNewConnCb _newconn_cb = nullptr;
    TcpConnectorDisConnCb _disconnected_cb = nullptr;
};

}


#endif // _TINYNET_TCP_CONNECTOR_H_