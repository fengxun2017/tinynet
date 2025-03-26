#ifndef _TINYNET_IO_SOCKET_H_
#define _TINYNET_IO_SOCKET_H_

#include <memory>
#include <string>
#include <netinet/in.h>
#include "socket_interface.h"

namespace tinynet
{

class IoSocket : public SocketInterface {

public:
    IoSocket(const std::string &name, Protocol protocol);

    ~IoSocket();

    bool bind(const std::string& self_ip, int self_port) override;
    bool listen(int backlog = 10) override;
    int accept(std::string& client_ip, int& client_port) override;
    int connect(struct sockaddr* addr, socklen_t addrlen) override;
    ssize_t write_data(const void* buffer, size_t length) override;
    /* only support for std can*/
    ssize_t write_can_data(uint32_t can_id, const void* buffer, size_t length) override;
    ssize_t read_data(void* buffer, size_t length) override;
    int get_fd() const override  {return _socket->get_fd();}
    Protocol get_protocol() const override {return _socket->get_protocol();}

    int get_socket_error() override;
    void close() override;

private:
    std::string _name;
    std::unique_ptr<SocketInterface> _socket;
};

}  // namespace tinynet

#endif  // _TINYNET_IO_SOCKET_H_