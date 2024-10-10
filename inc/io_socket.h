#ifndef _TINYNET_IO_SOCKET_H_
#define _TINYNET_IO_SOCKET_H_

#include <string>
#include <netinet/in.h>

namespace tinynet
{

class IoSocket {

public:
    enum Protocol { TCP, UDP };

    IoSocket(Protocol protocol);
    ~IoSocket();

    bool bind_socket(const std::string& self_ip, int self_port);
    bool listen_socket(int backlog = 10);
    int accept_socket();
    bool connect_socket(const std::string& remote_ip, int remote_port);
    ssize_t write_data(const void* buffer, size_t length);
    ssize_t read_data(void* buffer, size_t length);
    int get_fd(void) {return _sockfd;}
private:
    int _sockfd;
    Protocol _protocol;
    struct sockaddr_in _addr;
};


}  // namespace tinynet

#endif  // _TINYNET_IO_SOCKET_H_