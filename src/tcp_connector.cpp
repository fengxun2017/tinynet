#include <cstddef>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sstream>
#include <unistd.h>
#include "io_socket.h"
#include "tcp_connector.h"
#include "logging.h"
#include "tinynet_util.h"

namespace tinynet
{


TcpConnector::TcpConnector(EventLoop *event_loop, std::string name)
    : _name(name),_event_loop(event_loop)
{
    LOG(DEBUG) << _name << " created" << std::endl;
}

TcpConnector::~TcpConnector()
{
    LOG(DEBUG) << _name << "destructed" << std::endl;
}

void TcpConnector::connecting(void)
{
    _channel.reset(new IoChannel(_connector_socket->get_fd(), _event_loop->get_poller(), _name + ":channel"));
    // NOTE: If there is no NIC available, a writable event will be triggered only after 2 minutes for the connection failure to be detected.
    _channel->set_write_callback(std::bind(&TcpConnector::handle_write_complete, this));
    _channel->enable_write();
}

bool TcpConnector::connect(const std::string& server_ip, int server_port)
{
    bool ret;
    struct sockaddr_in server_addr;
    socklen_t addrlen = sizeof(server_addr);
    int _errno;
    static int create_index = 0;

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
    // FIXME: 当上一次正常的连接断开时（在tcpconnection的断开处理中关闭了套接字），如果又重新发起连接。则这里的new 创建IoSocket时，会拿到被关闭的套接字。
    // 而 reset 操作又会走当前_connector_socket的析构函数（里面有关闭套接字操作），导致新创建的IoSocket的套接字被关闭了。
    // 根因在于一个 socket，同时在多个模块中被管理，且相互不感知。
    _connector_socket.reset(new IoSocket(_name + ":socket_" + std::to_string(create_index++), IoSocket::TCP));
    state = _connector_socket->connect((struct sockaddr*)&server_addr, addrlen);
    /* 
       Currently, only non-blocking sockets are used. 
       Therefore, you need to obtain the error judgment status
    */
    _errno = (state == 0) ? 0 : errno;
    LOG(DEBUG) << _name << " connect return state:" << _errno << "{" << errno_str(_errno) << "}" << std::endl;
    switch (_errno)
    {
        case 0:
        case EISCONN:
        case EINPROGRESS:
            connecting();
            ret = true;
            break;

        default:
            _connector_socket->close();
            ret = false;
            LOG(ERROR) << _name << " connect to [" << server_ip << ":" << server_port << "] failed, err info:" << error_to_str(errno) << std::endl;
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
    _channel->disable_write();

    /* After epoll indicates writability, use getsockopt to read the SO_ERROR option at level SOL_SOCKET to determine whether
        connect() completed successfully (SO_ERROR is zero) or unsuccessfully 
    */
    ret = _connector_socket->get_socket_error();
    if ( 0 == ret) 
    {
        // connect success
        LOG(DEBUG) << _name << " connect success." << std::endl;
        if (0 == getsockname(_connector_socket->get_fd(), (struct sockaddr*)&local_addr, &local_addr_len))
        {
            if(NULL == inet_ntop(AF_INET, &(local_addr.sin_addr.s_addr), local_ip, sizeof(local_ip)))
            {
                LOG(WARNING) << "TcpConnector::connect getsockname failed, err info:" << error_to_str(errno) << std::endl;
            }
            local_port = ntohs(local_addr.sin_port);
        }
        if (0 == getpeername(_connector_socket->get_fd(), (struct sockaddr*)&peer_addr, &peer_addr_len))
        {
            if(NULL == inet_ntop(AF_INET, &(peer_addr.sin_addr.s_addr), peer_ip, sizeof(peer_ip)))
            {
                LOG(WARNING) << "TcpClient::connect getpeername failed, err info:" << error_to_str(errno) << std::endl;
            }
            peer_port = ntohs(peer_addr.sin_port);
        }
        std::ostringstream oss;
        oss << "[" << local_ip << ":" << local_port << "<->"
                <<  peer_ip << ":" << peer_port << "]";
        std::string conn_name = std::move(oss.str());
        new_conn = std::make_shared<TcpConnection>(_connector_socket->get_fd(), 
                local_ip, local_port, peer_ip, peer_port, _event_loop, conn_name);

        if (_newconn_cb)
        {
            _newconn_cb(new_conn);
        }
    } 
    else
    {
        LOG(ERROR) << _name << " connect failed, err info: " << error_to_str(ret) << std::endl;
        _channel->disable_all();
        _connector_socket->close();

        if (_disconnected_cb)
        {
            _disconnected_cb(nullptr);
        }
    }


}

}   // tinynet