
#include "conn_acceptor.h"

namespace tinynet
{

TcpAcceptor::TcpAcceptor(const std::string& ip, int port)
    : _io_socket(IoSocket::TCP), _ip(ip), _port(port) {}

TcpAcceptor::~TcpAcceptor() {}

bool TcpAcceptor::start() {
    if (!_io_socket.bind_socket(_ip, _port)) {
        std::cerr << "Failed to bind socket to " << _ip << ":" << _port << std::endl;
        return false;
    }
    if (!_io_socket.listen_socket()) {
        std::cerr << "Failed to listen on socket" << std::endl;
        return false;
    }
    return true;
}

int TcpAcceptor::accept_connection() {
    return _io_socket.accept_socket();
}

}  // namespace tinynet