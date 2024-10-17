#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "tcp_connector.h"
#include "logging.h"
#include "tinynet_util.h"

namespace tinynet
{


TcpConnector::TcpConnector(EventLoop *event_loop, std::string &name)
    : _name(name),
    _connector_socket(_name + ":socket", IoSocket::TCP),
    _event_loop(event_loop)
{
    LOG(DEBUG) << _name << "created" << std::endl;
}

TcpConnector::~TcpConnector()
{
    LOG(DEBUG) << _name << "destructed" << std::endl;
}

TcpConnPtr TcpConnector::connect(std::string& server_ip, int server_port)
{
    struct sockaddr_in local_addr;
    char local_ip[INET_ADDRSTRLEN] = {'U','N','K','N','O','W'};
    int local_port = 0;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    TcpConnPtr new_conn = nullptr;
    
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

}