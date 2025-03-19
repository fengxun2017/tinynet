#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/can.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/can/raw.h>
#include <netinet/tcp.h>
#include "linux_socket.h"
#include "tinynet_util.h"
#include "logging.h"
namespace tinynet
{

LinuxSocket::LinuxSocket(const std::string &name,  Protocol protocol)
: _name(name), _protocol(protocol), _sockfd(-1)
{
    int domain = -1;
    int type = -1;
    int proto = 0;
    int opt = 1;

    if (CAN == _protocol)
    {
        domain = AF_CAN;
        type = SOCK_RAW;
        proto = CAN_RAW;
    }
    else
    {
        if (TCP == _protocol) 
        {
            domain = AF_INET;
            type = SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC;
        } 
        else if (UDP == _protocol) {
            domain = AF_INET;
            type = SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC;
        }

    }
    _sockfd = socket(domain, type, protocol);
    if (_sockfd < 0)
    {
        LOG(ERROR) << "socket create failed, error info:" << error_to_str(errno) << std::endl;
        return ;
    }

    if (TCP == _protocol) 
    {
        // enable SO_KEEPALIVE
        int optval = 1;
        if (setsockopt(_sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
            LOG(ERROR) << "socket: "<< _name << " Failed to set SO_KEEPALIVE, error info:" << error_to_str(errno) << std::endl;
        }

        // The probe packet is sent after 5 seconds of idleness
        int keepIdle = 5;
        if (setsockopt(_sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(keepIdle)) < 0) {
            LOG(ERROR) << "socket: "<< _name << " Failed to set TCP_KEEPIDLE, error info:" << error_to_str(errno) << std::endl;
        }

        // A probe packet is sent every 2 seconds
        int keepInterval = 2; 
        if (setsockopt(_sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(keepInterval)) < 0) {
            LOG(ERROR) << "socket: "<< _name << " Failed to set TCP_KEEPINTVL, error info:" << error_to_str(errno) << std::endl;
        }

        // A maximum of 3 probe packets can be sent
        int keepCount = 3; 
        if (setsockopt(_sockfd, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(keepCount)) < 0) {
            LOG(ERROR) << "socket: "<< _name << " Failed to set TCP_KEEPCNT, error info:" << error_to_str(errno) << std::endl;
        }

        int opt = 1;
        if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
            LOG(ERROR) << "socket: "<< _name << " Failed to set SO_REUSEADDR, error info:" << error_to_str(errno) << std::endl;
        }
    }
}

LinuxSocket::~LinuxSocket()
{
    close();
    LOG(DEBUG) << "socket:"<< _name << " has been destructed.." << std::endl;
}

bool LinuxSocket::bind(const std::string& self_ip, int self_port) 
{
    bool ret = true;

    if (_protocol == Protocol::CAN)
    {
        struct ifreq ifr;
        strncpy(ifr.ifr_name, self_ip.c_str(), sizeof(ifr.ifr_name));

        // Get the device index
        ioctl(_sockfd, SIOCGIFINDEX, &ifr);
        struct sockaddr_can addr;
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;
        if (::bind(_sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
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
            if (::bind(_sockfd, (struct sockaddr*)&_addr, sizeof(_addr)) < 0)
            {
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

bool LinuxSocket::listen(int backlog) 
{
    bool ret = true;

    if (_protocol == Protocol::TCP)
    {
        if (::listen(_sockfd, backlog) < 0)
        {
            LOG(ERROR) << "listen failed, err info:" << error_to_str(errno);
            ret = false;
        }
    }

    return ret;
}

int LinuxSocket::accept(std::string& client_ip, int& client_port)  {
    int new_sock = -1;

    if (_protocol == Protocol::TCP)
    {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        new_sock = accept4(_sockfd, (struct sockaddr*)&client_addr, &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (new_sock < 0)
        {
            LOG(ERROR) << "Accept failed, err info:" << error_to_str(errno);
        }
        else
        {
            char ipv4_str[INET_ADDRSTRLEN] = {0};
            if (NULL != inet_ntop(AF_INET, &(client_addr.sin_addr.s_addr), ipv4_str, INET_ADDRSTRLEN))
            {
                client_ip = std::string(ipv4_str);
                client_port = ntohs(client_addr.sin_port);
            }
        }
    }

    return new_sock;
}

int LinuxSocket::connect(struct sockaddr* addr, socklen_t addrlen) 
{
    return ::connect(_sockfd, addr, addrlen);
}

ssize_t LinuxSocket::write_data(const void* buffer, size_t length) 
{
    if (_protocol == Protocol::TCP)
    {
        return send(_sockfd, buffer, length, 0);
    }
    return -1;
}

ssize_t LinuxSocket::write_can_data(uint32_t can_id, const void* buffer, size_t length) 
{
    ssize_t ret = -1;

    if (_protocol == Protocol::CAN)
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
            ret = ::write(_sockfd, &frame, sizeof(frame));
        }
    }
    LOG(ERROR) << "this api only support write can data" << std::endl;
    return -1;
}

ssize_t LinuxSocket::read_data(void* buffer, size_t length) 
{
    ssize_t ret = -1;
    if (_protocol == Protocol::TCP) {
        ret = recv(_sockfd, buffer, length, 0);
    }

    return ret;
}

int LinuxSocket::get_fd() const 
{
    return _sockfd;
}

int LinuxSocket::get_socket_error() 
{
    int optval;
    socklen_t optlen = sizeof(optval);

    if (::getsockopt(_sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        LOG(ERROR) << "get_socket_error failed, err info:" << error_to_str(errno);
        optval = errno;
    }

    return optval;
}

void LinuxSocket::close()  {
    if (check_fd(_sockfd))
    {
        ::close(_sockfd);
        _sockfd = -1;
    }
}

}  // namespace tinynet