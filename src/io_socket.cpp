#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include "io_socket.h"
#include "tinynet_util.h"
#include "logging.h"
#include "socket_factory.h"

namespace tinynet
{
IoSocket::IoSocket(const std::string &name, Protocol protocol) 
: _name(name)
{
    _socket = SocketFactory::create_socket(name, protocol); 
}

IoSocket::~IoSocket() {
    close();
    LOG(DEBUG) << "IoSocket:" <<_name << " has been destructed.." << std::endl;
}

void IoSocket::close(void) {
    _socket->close();
}

bool IoSocket::bind(const std::string& self_ip, int self_port) {
    return _socket->bind(self_ip, self_port);
}

bool IoSocket::listen(int backlog) {
    return _socket->listen(backlog);
}

int IoSocket::accept(std::string& client_ip, int& client_port) {
    return _socket->accept(client_ip, client_port);
}

int IoSocket::connect(struct sockaddr* addr, socklen_t addrlen) {
    return _socket->connect(addr, addrlen);
}

ssize_t IoSocket::write_data(const void* buffer, size_t length) {
    return _socket->write_data(buffer, length);
}

ssize_t IoSocket::write_can_data(uint32_t can_id, const void* buffer, size_t length) {
    return _socket->write_can_data(can_id, buffer, length);
}

ssize_t IoSocket::read_data(void* buffer, size_t length) {
    return _socket->read_data(buffer, length);
}

int IoSocket::get_socket_error(void) {
    return _socket->get_socket_error();
}

}  // namespace tinynet