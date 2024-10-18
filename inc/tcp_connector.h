#ifndef _TINYNET_TCP_CONNECTOR_H_
#define _TINYNET_TCP_CONNECTOR_H_

#include <string>
#include "io_socket.h"
#include "io_channel.h"
#include "event_loop.h"
#include "tcp_connection.h"
namespace tinynet
{

class TcpConnector {
public:
    TcpConnector(EventLoop *event_loop, std::string name);

    ~TcpConnector();

    void connect(std::string& server_ip, int server_port);

private:
    void handle_write_complete(void);

    std::string _name;
    IoSocket _connector_socket;
    EventLoop *_event_loop;
    IoChannel _channel;
};

}


#endif // _TINYNET_TCP_CONNECTOR_H_