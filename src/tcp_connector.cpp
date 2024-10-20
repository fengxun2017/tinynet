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

bool TcpConnector::connect(std::string& server_ip, int server_port)
{
    bool ret;
    struct sockaddr_in server_addr;
    socklen_t addrlen = sizeof(server_addr);

    _server_ip = server_ip;
    _server_port = server_port;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    int state = inet_pton(AF_INET, server_ip.c_str(), &(server_addr.sin_addr.s_addr));
    if (0 >= state)
    {
        LOG(ERROR) << "IP:" << server_ip << " is not a valid network address. " 
                << "or af does not contain a valid address family." << std::endl;
        return false;
    }

    state = _connector_socket.connect_socket((struct sockaddr*)&server_addr, addrlen);
    switch (state)
    {
        case 0:
        case EINPROGRESS:
            _channel.set_write_callback(std::bind(&TcpConnector::handle_write_complete, this));
            _channel.enable_write();
            ret = true;
            break;

        default:
            ret = false;
            LOG(ERROR) << _name << " connect to [" << server_ip << ":" << server_port << "] failed, err info:" << error_to_str(errno);
            _connector_socket.close();
            break;

    }
    return ret;
}

void TcpConnector::handle_write_complete(void)
{
    int ret;
    TcpConnPtr new_conn = nullptr;
    struct sockaddr_in local_addr;
    char local_ip[INET_ADDRSTRLEN] = {'U','N','K','N','O','W'};
    int local_port = 0;
    socklen_t local_addr_len = sizeof(struct sockaddr_in);

    struct sockaddr_in peer_addr;
    char peer_ip[INET_ADDRSTRLEN] = {'U','N','K','N','O','W'};
    int peer_port = 0;
    socklen_t peer_addr_len = sizeof(struct sockaddr_in);

    LOG(DEBUG) << _name << " handle_write_complete" << std::endl;
    _channel.disable_write();

    ret = _connector_socket.get_socket_error();
    if ( 0 == ret) 
    {
        // connect success
        if (0 == getsockname(_connector_socket.get_fd(), (struct sockaddr*)&local_addr, &local_addr_len))
        {
            if(NULL == inet_ntop(AF_INET, &(local_addr.sin_addr.s_addr), local_ip, sizeof(local_ip)))
            {
                LOG(WARNING) << "TcpConnector::connect getsockname failed, err info:" << error_to_str(errno) << std::endl;
            }
            local_port = ntohs(local_addr.sin_port);
        }
        if (0 == getpeername(_connector_socket.get_fd(), (struct sockaddr*)&peer_addr, &peer_addr_len))
        {
            if(NULL == inet_ntop(AF_INET, &(peer_addr.sin_addr.s_addr), peer_ip, sizeof(peer_ip)))
            {
                LOG(WARNING) << "TcpClient::connect getpeername failed, err info:" << error_to_str(errno) << std::endl;
            }
            peer_port = ntohs(peer_addr.sin_port);
        }

        new_conn = std::make_shared<TcpConnection>(_connector_socket.get_fd(), 
                local_ip, local_port, peer_ip, peer_port, _event_loop, _name+":conn");
        if (_newconn_cb)
        {
            _newconn_cb(new_conn);
        }
    } 
    else
    {
        LOG(ERROR) << _name << " connect failed, err info: " << error_to_str(ret);
        if (_disconnected_cb)
        {
            _disconnected_cb(nullptr);
        }
        
    }



}