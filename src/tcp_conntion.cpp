#include <string>
#include "tcp_conntion.h"
#include "logging.h"

namespace tinynet
{


TcpConnection::TcpConnection(int sockfd, const std::string& client_ip, int client_port)
    : _sockfd(sockfd), _client_ip(client_ip), _client_port(client_port)
{

}

TcpConnection::~TcpConnection() {
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

void TcpConnection::handle_recvdata(uint8_t data, size_t len)
{

}

}