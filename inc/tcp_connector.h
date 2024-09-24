#ifndef _TINYNET_TCP_CONNECTOR_H_
#define _TINYNET_TCP_CONNECTOR_H_

#include <string>
#include "io_socket.h"
#include "io_channel.h"
#include "event_loop.h"

namespace tinynet
{

class TcpConnector {
public:
    TcpConnector(EventLoop *event_loop, std::string &name, std::string& server_ip, int server_port)
        : _name(name),_connector_socket(IoSocket::TCP),
          _remote_ip(server_ip), _remote_port(server_port),
          _event_loop(event_loop), 
          _channel(_connector_socket.get_fd(), _event_loop->get_poller(), _name)
    {
        
    }

    ~TcpConnector() {}

    bool connect() {
        return _connector_socket.connect_socket(_remote_ip, _remote_port);
    }

    ssize_t write_data(const void* buffer, size_t length) {
        return _connector_socket.write_data(buffer, length);
    }

    ssize_t read_data(void* buffer, size_t length) {
        return _connector_socket.read_data(buffer, length);
    }

private:
    std::string _name;
    IoSocket _connector_socket;
    std::string _remote_ip;
    int _remote_port;
    EventLoop *_event_loop;
    IoChannel _channel;
};

}


#endif // _TINYNET_TCP_CONNECTOR_H_