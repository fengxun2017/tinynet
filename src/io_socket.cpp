#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include "io_socket.h"
#include "tinynet_util.h"
#include "logging.h"
#include <linux/can.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/can/raw.h>

namespace tinynet
{

IoSocket::IoSocket(std::string name, Protocol protocol) : _name(name)
{
    int opt = 1;
    _protocol = protocol;
    _sockfd = -1;

    if (CAN == _protocol)
    {
        _sockfd = socket(AF_CAN, SOCK_RAW, CAN_RAW);
    }
    else
    {
        if (TCP == _protocol) 
        {
            _sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
        } 
        else if (UDP == _protocol) {
            _sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
        }
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    if (_sockfd < 0) {
        LOG(ERROR) << "socket: "<< _name << " create failed, error info:" << error_to_str(errno) << std::endl;
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

    if (CAN == _protocol)
    {
        struct ifreq ifr;
        strncpy(ifr.ifr_name, self_ip.c_str(), sizeof(ifr.ifr_name));

        // Get the device index
        ioctl(_sockfd, SIOCGIFINDEX, &ifr); 
        struct sockaddr_can addr;
        addr.can_family = AF_CAN;  
        addr.can_ifindex = ifr.ifr_ifindex;  
         if (bind(_sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
         {
            LOG(ERROR) << "bind failed, error info:" << error_to_str(errno) << std::endl;
            ret = false;
         }
    }
    else
    {
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

int IoSocket::connect_socket(struct sockaddr* addr, socklen_t addrlen)
{
    int ret = 0;

    ret = connect(_sockfd, addr, addrlen);

    return ret;
}

ssize_t IoSocket::write_data(const void* buffer, size_t length)
{
    ssize_t ret = -1;
    if (TCP == _protocol)
    {
        ret = send(_sockfd, buffer, length, 0);
    } 
    // else if (protocol == UDP) {
    //     return sendto(sockfd, buffer, length, 0, (struct sockaddr*)&_addr, sizeof(_addr));
    // }

    return ret;
}


ssize_t IoSocket::write_can_data(uint32_t can_id, const void* buffer, size_t length)
{
    ssize_t ret = -1;

    if (CAN == _protocol)
    {
        if (length > 8)
        {
            LOG(ERROR) << "only support std can" << std::endl;
        }
        else
        {
            struct can_frame frame;
            memcpy(frame.data, buffer, length);
            
            frame.can_dlc = length;
            frame.can_id = can_id;
            write(_sockfd, &frame, sizeof(frame));
        }
    }
    else 
    {
        LOG(ERROR) << "this api only support write can data" << std::endl;
    }

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