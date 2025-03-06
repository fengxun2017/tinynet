
#include <netinet/in.h>
#include <cerrno>
#include <sstream>
#include <functional>
#include <unistd.h>
#include "tinynet_util.h"
#include "tcp_acceptor.h"
#include "tcp_connection.h"
#include "logging.h"

namespace tinynet
{

TcpAcceptor::TcpAcceptor(EventLoop *event_loop, const std::string& ip, int port, std::string name)
    : _name(name),
      _event_loop(event_loop),
      _acceptor_socket(_name + ":socket", IoSocket::TCP),
      _ip(ip),
      _port(port),
      _channel(_acceptor_socket.get_fd(), _event_loop->get_poller(), _name + ":channel")
{
    LOG(DEBUG) << "TcpAcceptor created: " << _name << std::endl;
}

TcpAcceptor::~TcpAcceptor()
{
    LOG(DEBUG) << _name << " has been destructed." << std::endl;
}

bool TcpAcceptor::start() 
{
    if (!_acceptor_socket.bind_socket(_ip, _port)) {
        std::cerr << "Failed to bind socket to " << _ip << ":" << _port << std::endl;
        return false;
    }
    LOG(DEBUG) << "bind to [" << _ip << ":" << _port << "] success" << std::endl;

    if (!_acceptor_socket.listen_socket()) {
        std::cerr << "Failed to listen on socket" << std::endl;
        return false;
    }
    LOG(DEBUG) << "listen on [" << _ip << ":" << _port << "] success" << std::endl;

    _channel.set_read_callback(std::bind(&TcpAcceptor::accept_connection, this));
    _channel.enable_read();

    // LOG(INFO) << "server: [" << _ip << ":" << _port << "] start!" << std::endl;
    return true;
}

void TcpAcceptor::close() 
{
    LOG(INFO) << "close " << _name << std::endl;
    _channel.disable_read();
}

void TcpAcceptor::accept_connection(void)
{
    TcpConnPtr new_conn = nullptr;
    // struct sockaddr_in client_addr;
    // socklen_t client_len = sizeof(client_addr);
    std::string client_ip = "UNKNOW";
    int client_port = 0;
    int client_sockfd = _acceptor_socket.accept_socket(client_ip, client_port);

    if (check_fd(client_sockfd))
    {
#ifdef TINYNET_DEBUG
     check_fd_nonblock(client_sockfd);
#endif
        if (nullptr != _newconn_cb)
        {
            _newconn_cb(client_sockfd, client_ip, client_port);
        }
    }
    else
    {
        LOG(ERROR) << "accept failed in TcpAcceptor::accept_connection, err info:" << error_to_str(errno); 
    }
}

}  // namespace tinynet