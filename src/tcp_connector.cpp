#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "tcp_connector.h"
#include "logging.h"
#include "tinynet_util.h"

namespace tinynet
{


TcpConnector::TcpConnector(EventLoop *event_loop, std::string name)
    : _name(name),
    _connector_socket(_name + ":socket", IoSocket::TCP),
    _event_loop(event_loop),
    _channel(_connector_socket.get_fd(), event_loop->get_poller(), _name + ":channel"),
{
    LOG(DEBUG) << _name << " created" << std::endl;
}

TcpConnector::~TcpConnector()
{
    LOG(DEBUG) << _name << "destructed" << std::endl;
}

void TcpConnector::connect(std::string& server_ip, int server_port)
{
    struct sockaddr_in local_addr;
    char local_ip[INET_ADDRSTRLEN] = {'U','N','K','N','O','W'};
    int local_port = 0;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    TcpConnPtr new_conn = nullptr;

    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    int state = inet_pton(AF_INET, server_ip.c_str(), &(server_addr.sin_addr.s_addr));
    if (0 >= state)
    {
        LOG(ERROR) << "IP:" << server_ip << " is not a valid network address. " 
                << "or af does not contain a valid address family." << std::endl;
        return ;
    }

    ret = _connector_socket.connect_socket(server_addr);
    switch (ret)
    {
        case 0:
        case EINPROGRESS:
            channel.set_write_callback(std::bind(&TcpConnector::handle_write_complete, this));
            chennel.enable_read();
            break;
        
        default:
            LOG(ERROR) << _name " connect to [" << server_ip << ":" << server_port << "] failed, err info:" << error_to_str(errno); 
            break;

    }
    bool ret =  _connector_socket.connect_socket(server_ip, server_port);
    if (ret)
    {
        if (0 == getsockname(_connector_socket.get_fd(), (struct sockaddr*)&local_addr, &addr_len))
        {
            if(NULL == inet_ntop(AF_INET, &(local_addr.sin_addr.s_addr), local_ip, sizeof(local_ip)))
            {
                LOG(WARNING) << "TcpConnector::connect getsockname failed, err info:" << error_to_str(errno) << std::endl;
            }
            local_port = ntohs(local_addr.sin_port);
        }    
        new_conn = std::make_shared<TcpConnection>(_connector_socket.get_fd(), 
                local_ip, local_port, server_ip, server_port, _event_loop, _name+":conn");
    }

    return new_conn;
}

void TcpConnector::handle_write_complete(void)
{
    LOG(DEBUG) << _name << " handle_write_complete" << std::endl;
        getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
        if (so_error == 0) {
            // 连接成功
        } else {
            // 连接失败，错误码是so_error

        }


}