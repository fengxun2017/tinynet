#ifndef _TINYNET_LINUX_SOCKET_H_
#define _TINYNET_LINUX_SOCKET_H_

#include <sys/socket.h>
#include <string>
#include "socket_interface.h"

namespace tinynet
{

class LinuxSocket : public SocketInterface {
public:
    LinuxSocket(const std::string &name,  Protocol protocol);
    ~LinuxSocket() override;

    bool bind(const std::string& self_ip, int self_port) override;
    bool listen(int backlog) override;
    int accept(std::string& client_ip, int& client_port) override;
    int connect(struct sockaddr* addr, socklen_t addrlen) override;
    ssize_t write_data(const void* buffer, size_t length) override;
    ssize_t write_can_data(uint32_t can_id, const void* buffer, size_t length) override;
    ssize_t read_data(void* buffer, size_t length) override;
    int get_fd() const override;
    Protocol get_protocol() const override;
    int get_socket_error() override;
    void close() override;

private:
    int _sockfd;
    struct sockaddr_in _addr;
    Protocol _protocol;
    std::string _name;
};

}  // namespace tinynet

#endif  // _TINYNET_LINUX_SOCKET_H_