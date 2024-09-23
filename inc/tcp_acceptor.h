#ifndef _TINYNET_TCP_ACCEPTOR_H_
#define _TINYNET_TCP_ACCEPTOR_H_
#include <iostream>
#include <string>
#include "io_socket.h"

namespace tinynet
{

class TcpAcceptor {
public:
    TcpAcceptor(const std::string& ip, int port);
    ~TcpAcceptor();

    bool start();
    int accept_connection();

private:
    IoSocket _acceptor_socket;
    std::string _ip;
    int _port;
};

}  // namespace tinynet

#endif  // _TINYNET_CONN_ACCEPTOR_H_
