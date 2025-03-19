#ifndef _TINYNET_SOCKET_INTERFACE_H_
#define _TINYNET_SOCKET_INTERFACE_H_

#include <string>
#include <netinet/in.h>

namespace tinynet
{

class SocketInterface {
public:
    enum Protocol { TCP, UDP, CAN };
    virtual ~SocketInterface() {}

    virtual bool bind(const std::string& self_ip, int self_port) = 0;
    virtual bool listen(int backlog) = 0;
    virtual int accept(std::string& client_ip, int& client_port) = 0;
    virtual int connect(struct sockaddr* addr, socklen_t addrlen) = 0;
    virtual ssize_t write_data(const void* buffer, size_t length) = 0;
    virtual ssize_t write_can_data(uint32_t can_id, const void* buffer, size_t length) = 0;
    virtual ssize_t read_data(void* buffer, size_t length) = 0;
    virtual int get_fd() const = 0;
    virtual int get_socket_error() = 0;
    virtual void close() = 0;
};

}  // namespace tinynet

#endif  // _TINYNET_SOCKET_INTERFACE_H_