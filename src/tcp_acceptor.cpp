
#include <netinet/in.h>
#include <cerrno>
#include "tinynet_util.h"
#include "tcp_acceptor.h"
#include "tcp_connection.h"
#include "logging.h"
#include <functional>

namespace tinynet
{

TcpAcceptor::TcpAcceptor(EventLoop *event_loop, const std::string& ip, int port, std::string name)
    : _name(name),
      _event_loop(event_loop),
      _acceptor_socket(IoSocket::TCP),
      _ip(ip),
      _port(port),
      _channel(_acceptor_socket.get_fd(), _event_loop->get_poller(), _name + "_channel")
{

}

TcpAcceptor::~TcpAcceptor() {}

bool TcpAcceptor::start() 
{
    if (!_acceptor_socket.bind_socket(_ip, _port)) {
        std::cerr << "Failed to bind socket to " << _ip << ":" << _port << std::endl;
        return false;
    }

    LOG(DEBUG) << "bind to" << _ip << ":" << _port << std::endl;
    if (!_acceptor_socket.listen_socket()) {
        std::cerr << "Failed to listen on socket" << std::endl;
        return false;
    }
    LOG(DEBUG) << "skcket listen success" << std::endl;

    _channel.set_reab_callback(std::bind(&TcpAcceptor::accept_connection, this));
    _channel.enable_read();

    return true;
}

void TcpAcceptor::accept_connection(void)
{
    std::shared_ptr<TcpConnection> new_conn = nullptr;
    int client_sockfd = _acceptor_socket.accept_socket();
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    std::string client_ip = "UNKNOW";
    int client_port = 0;

    if (check_fd(client_sockfd))
    {
        if (0 == getpeername(client_sockfd, (struct sockaddr*)&client_addr, &client_len))
        {
            char ipv4_str[INET_ADDRSTRLEN] = {0};
            if(NULL != inet_ntop(AF_INET, &(client_addr.sin_addr.s_addr), ipv4_str, INET_ADDRSTRLEN))
            {
                client_ip = std::string(ipv4_str);
                client_port = ntohs(client_addr.sin_port);
            }
        }
        LOG(INFO) << "server:[" << _ip << ":" << _port << "] receives a connection request from client:[]"
                <<  client_ip << ":" << client_port << "]" << std::endl;
        new_conn = std::make_shared<TcpConnection>(client_sockfd, client_ip, client_port, _ip, _port);

    }
    else
    {
        LOG(ERROR) << "accept failed in TcpAcceptor::accept_connection, err info:" << error_to_str(errno); 
    }

    if (nullptr != _newconn_cb && nullptr != new_conn)
    {
        _newconn_cb(new_conn);
    }
}

}  // namespace tinynet