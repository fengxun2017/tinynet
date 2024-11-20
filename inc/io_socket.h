#ifndef _TINYNET_IO_SOCKET_H_
#define _TINYNET_IO_SOCKET_H_

#include <string>
#include <netinet/in.h>

namespace tinynet
{

class IoSocket {

public:
    enum Protocol { TCP, UDP, CAN };

    IoSocket(std::string name, Protocol protocol);
    ~IoSocket();

    bool bind_socket(const std::string& self_ip, int self_port);
    bool listen_socket(int backlog = 10);
    int accept_socket(std::string& client_ip, int& client_port);
    int connect_socket(struct sockaddr* addr, socklen_t addrlen);
    ssize_t write_data(const void* buffer, size_t length);

    /* only support for std can*/
    ssize_t write_can_data(uint32_t can_id, const void* buffer, size_t length);
    ssize_t read_data(void* buffer, size_t length);
    int get_fd(void) {return _sockfd;}
    int get_socket_error(void);
    void close(void);
private:
    std::string _name;
    int _sockfd;
    Protocol _protocol;
    struct sockaddr_in _addr;
};


}  // namespace tinynet

#endif  // _TINYNET_IO_SOCKET_H_