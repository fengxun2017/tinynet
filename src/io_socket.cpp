#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include "io_socket.h"
#include "tinynet_util.h"
#include "logging.h"

namespace tinynet
{

IoSocket::IoSocket(std::string name, Protocol protocol) : _name(name)
{
    _protocol = protocol;
    _sockfd = -1;

    if (TCP == _protocol) 
    {
        _sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    } 
    else if (UDP == _protocol) {
        _sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    }

    if (_sockfd < 0) {
        LOG(ERROR) << "socket create failed, error info:" << error_to_str(errno) << std::endl;
    }
}

IoSocket::~IoSocket()
{
    close();
    LOG(DEBUG) << _name << " has been destructed.." << std::endl;
}

void IoSocket::close(void)
{
    if (check_fd(_sockfd)) 
    {
        ::close(_sockfd);
        _sockfd = -1;
    }
}


bool IoSocket::bind_socket(const std::string& self_ip, int self_port)
{
    bool ret = true;

    memset(&_addr, 0, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(self_port);
    
    int s = inet_pton(AF_INET, self_ip.c_str(), &(_addr.sin_addr.s_addr));
    if (1 == s)
    {
        if (bind(_sockfd, (struct sockaddr*)&_addr, sizeof(_addr)) < 0) {
            LOG(ERROR) << "bind failed, error info:" << error_to_str(errno) << std::endl;
            ret = false;
        }
    }
    else
    {
        ret = false;
        if (0 == s)
        {
            LOG(ERROR) << "IP:" << self_ip << " is not a valid network address" << std::endl;
        }
        else
        {
            LOG(ERROR) << "inet_pton failed, err info:" << error_to_str(errno) << std::endl;
        }
    }

    return ret;
}

bool IoSocket::listen_socket(int backlog)
{
    bool ret = true;

    if (_protocol == TCP) {
        if (listen(_sockfd, backlog) < 0) {
            LOG(ERROR) << "listen failed, err info:" << error_to_str(errno);
            ret = false;
        }
    }

    return ret;
}

int IoSocket::accept_socket(std::string& client_ip, int& client_port)
{
    int new_sock = -1;

    if (_protocol == TCP) 
    {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        new_sock = accept4(_sockfd, (struct sockaddr*)&client_addr, &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (new_sock < 0) {
            LOG(ERROR) << "Accept failed, err info:" << error_to_str(errno);
        }
        else
        {
            char ipv4_str[INET_ADDRSTRLEN] = {0};
            if(NULL != inet_ntop(AF_INET, &(client_addr.sin_addr.s_addr), ipv4_str, INET_ADDRSTRLEN))
            {
                client_ip = std::string(ipv4_str);
                client_port = ntohs(client_addr.sin_port);
            }
        }
    }

    return new_sock;
}

int IoSocket::connect_socket(struct sockaddr* addr, socklen_t addrlen) {
    int ret = 0;

    ret = connect(_sockfd, addr, addrlen);

    return ret;
}

ssize_t IoSocket::write_data(const void* buffer, size_t length) {
    if (_protocol == TCP)
    {
        return send(_sockfd, buffer, length, 0);
    } 
    // else if (protocol == UDP) {
    //     return sendto(sockfd, buffer, length, 0, (struct sockaddr*)&_addr, sizeof(_addr));
    // }

    return -1;
}

ssize_t IoSocket::read_data(void* buffer, size_t length) {
    if (_protocol == TCP)
    {
        return recv(_sockfd, buffer, length, 0);
    } 
    // else if (protocol == UDP) {
    //     socklen_t addr_len = sizeof(addr);
    //     return recvfrom(sockfd, buffer, length, 0, (struct sockaddr*)&addr, &addr_len);
    // }

    return -1;
}

int IoSocket::get_socket_error(void)
{
    int optval;
    socklen_t optlen = sizeof(optval);

    if (::getsockopt(_sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        LOG(ERROR) << "IoSocket::get_socket_error failed, err info:" << error_to_str(errno);
        optval = errno;
    }

    return optval;
}

}  // namespace tinynet