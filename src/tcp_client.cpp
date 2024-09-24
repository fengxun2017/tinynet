#include <sys/epoll.h>
#include <arpa/inet.h>
#include <string>
#include "tcp_client.h"
#include "logging.h"
#include "tinynet_util.h"
namespace tinynet
{
TcpClient::TcpClient(EventLoop *event_loop, std::string client_name) :
    _name(client_name), 
    _event_loop(event_loop),
    _connector(event_loop, client_name + ":connector")
{

    LOG(DEBUG) << "client:" << _name << " has been created" << std::endl;
}

TcpClient::~TcpClient()
{
    
    LOG(DEBUG) << "client:" << _name << " has been destructed" << std::endl;
}

bool TcpClient::connect(std::string server_ip, int server_port)
{
    struct sockaddr_in peer_addr;
    char peer_ip[INET_ADDRSTRLEN] = {'U','N','K','N','O','W'};
    int peer_port = 0;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    bool ret = false;

    if (nullptr == _conn)
    {
        _conn = _connector.connect(server_ip, server_port);
        if (nullptr != _conn)
        {
            LOG(INFO) << _name << " connect to " << server_ip << ":" << server_port << " success" << std::endl;
            ret = true;
        }
    }
    else
    {
        if (0 == getpeername(_conn->get_fd(), (struct sockaddr*)&peer_addr, &addr_len))
        {
            if(NULL == inet_ntop(AF_INET, &(peer_addr.sin_addr.s_addr), peer_ip, sizeof(peer_ip)))
            {
                LOG(WARNING) << "TcpClient::connect getpeername failed, err info:" << error_to_str(errno) << std::endl;
            }
            peer_port = ntohs(peer_addr.sin_port);
        }
        LOG(INFO) << _name << " is already connected to " << peer_ip << ":" << peer_port << std::endl;
    }

    if (ret && nullptr != _conn)
    {
        _conn->set_disconnected_cb(std::bind(&TcpClient::handle_disconnected, this, std::placeholders::_1));
        _conn->set_onmessage_cb(std::bind(&TcpClient::handle_message, this,
                                std::placeholders::_1,
                                std::placeholders::_2,
                                std::placeholders::_3));
        _conn->enable_read();

        handle_new_connection(_conn);
    }

    return ret;
}

void TcpClient::handle_new_connection(TcpConnPtr conn)
{
    if (_newconn_cb)
    {
        _newconn_cb(conn);
    }
}

void TcpClient::handle_disconnected(TcpConnPtr conn)
{
    if (_disconnected_cb)
    {
        _disconnected_cb(conn);
    }
}

void TcpClient::handle_message(TcpConnPtr conn, const uint8_t *data, size_t size)
{
    if (_on_message_cb)
    {
        _on_message_cb(conn, data, size);
    }
}
}