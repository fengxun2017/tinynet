#include <string>
#include "tcp_connection.h"
#include "logging.h"

namespace tinynet
{

TcpConnection::TcpConnection(int sockfd, const std::string& client_ip, int client_port,
                const std::string& server_ip, int server_port,
                EventLoop *event_loop, std::string &name);
    : _name(name),
      _sockfd(sockfd),
      _client_ip(client_ip),
      _client_port(client_port),
      _server_ip(server_ip),
      _server_port(server_port),
      _channel(sockfd, event_loop->get_poller(), _name + "_channel")

{
    _channel.set_reab_callback
    _channel.set_write_callback
    _channel.set_close_callback
    _channel.set_error_callback

}

TcpConnection::~TcpConnection() 
{
    close(_sockfd);
}

void TcpConnection::write_data(const void* buffer, size_t length)
{

}


std::string TcpConnection::get_client_ip(void) 
{
    return _client_ip;
}

int TcpConnection::get_client_port(void) 
{
    return _client_port;
}

void onmessage_handler(void)
{

}
void disconnected_handler(void)
{

}
void write_complete_handler(void)
{

}

} // tinynet
